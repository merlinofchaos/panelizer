<?php
// $Id: $

/**
 * @file
 * Contains routines specific to handling nodes that are panelized.
 */

/**
 * Retrieve the panelizer panel associated with a node.
 */
function panelizer_load_node_panelizer($node, $default_anyway = FALSE) {
  if (!empty($node) && is_numeric($node)) {
    $node = node_load($node);
  }

  if (!$node) {
    return FALSE;
  }

  if (!panelizer_is_panelized('node', $node->type)) {
    return FALSE;
  }

  $panelizer = db_fetch_object(db_query("SELECT * FROM {panelizer_node} WHERE nid = %d", $node->nid));
  if ($panelizer && !empty($panelizer->did)) {
    ctools_include('export');
    $panelizer = ctools_export_unpack_object('panelizer_node', $panelizer);
    $panelizer->panelizer_type = 'node';
    return $panelizer;
  }

  // TODO: When we implement choice, we would look to see which default is in use.
  $choice = 'default';

  // Is there a default?
  $panelizer = panelizer_load_default('node', $node->type, $choice, $default_anyway);
  if (!$panelizer) {
    return FALSE;
  }

  // To ensure this saves properly later, we need to scrub out the data that
  // is specific to the default.
  unset($panelizer->name);
  unset($panelizer->title);
  $original_display = panelizer_load_display($panelizer);

  // In order to ensure we don't actually use and modify the default display,
  // we export and re-import it.
  $code = panels_export_display($original_display);
  ob_start();
  eval($code);
  ob_end_clean();

  $panelizer->display = $display;
  $panelizer->did = NULL;
  $panelizer->nid = $node->nid;
  $panelizer->new = TRUE;

  return $panelizer;
}

/**
 * Write a panelizer node panel to the database.
 */
function panelizer_save_node_panelizer($panelizer) {
  if (!empty($panelizer->display)) {
    // First write the display
    panels_save_display($panelizer->display);

    // Make sure we have the did.
    // @todo -- this should never spontaneously change on us, but we aren't
    // checking that, either. We possibly should enforce a safety condition
    // here.
    $panelizer->did = $panelizer->display->did;
  }

  // Then write the default
  if (empty($panelizer->new)) {
    // Existing record.
    $update = array('nid');
  }
  else {
    // New record.
    $update = array();
  }

  return drupal_write_record('panelizer_node', $panelizer, $update);
}

/**
 * Page to edit basic settings on a panelized node.
 */
function panelizer_edit_node_settings_page($node) {
  $panelizer = panelizer_load_node_panelizer($node);
  if (empty($panelizer)) {
    // If this node is not yet panelized, and there is no default panel
    // do to configuration, give them the option of panelizing it.
    if (panelizer_has_default('node', $node->type)) {
      return MENU_NOT_FOUND;
    }

    // Set the form to the Panelize It! form.
    $form_id = 'panelizer_panelize_node_form';

    // Fetch a special default panelizer that is only accessible with the
    // default_anyway flag.
    $panelizer = panelizer_load_node_panelizer($node, TRUE);
  }
  else {
    $form_id = 'panelizer_settings_form';
  }

  $form_state = array(
    'panelizer' => &$panelizer,
    'no_redirect' => TRUE,
  );

  ctools_include('form');
  ctools_include('common', 'panelizer');
  $output = ctools_build_form($form_id, $form_state);
  if (!empty($form_state['executed'])) {
    drupal_set_message(t('The settings have been updated.'));
    panelizer_save_node_panelizer($panelizer);
    drupal_goto($_GET['q']);
  }

  return $output;
}

function panelizer_panelize_node_form(&$form_state) {
  $form = array();

  $form['markup'] = array(
    '#value' => '<p>' . t('This node is not currently panelized.') . '</p>',
  );

  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Panelize it!'),
  );

  return $form;
}

/**
 * Page to edit basic settings on a panelized node.
 */
function panelizer_edit_node_context_page($node) {
  $cache_key = $node->nid;
  $panelizer = panelizer_context_cache_get('node', $cache_key);
  if (empty($panelizer)) {
    return MENU_NOT_FOUND;
  }

  $form_state = array(
    'panelizer' => &$panelizer,
    'panelizer type' => 'node',
    'panelizer id' => 'node',
    'cache key' => $cache_key,
    'no_redirect' => TRUE,
  );

  ctools_include('form');
  ctools_include('common', 'panelizer');
  $output = ctools_build_form('panelizer_default_context_form', $form_state);
  if (!empty($form_state['executed'])) {
    if (!empty($form_state['clicked_button']['#write'])) {
      drupal_set_message(t('The settings have been updated.'));
      panelizer_save_node_panelizer($panelizer);
    }
    else {
      drupal_set_message(t('Changes have been discarded.'));
    }

    panelizer_context_cache_clear('node', $cache_key);
    drupal_goto($_GET['q']);
  }

  return $output;
}

/**
 * Page to edit basic settings on a panelized node.
 */
function panelizer_edit_node_layout_page($node) {
  $panelizer = panelizer_load_node_panelizer($node);
  if (empty($panelizer)) {
    return MENU_NOT_FOUND;
  }

  $display = panelizer_load_display($panelizer);

  ctools_include('context');
  $display->context = panelizer_get_contexts($panelizer, $node);
  $output = panels_edit_layout($display, t('Save'), '', 'panelizer_node' . ':' . $node->type);
  if (is_object($output)) {
    // The display was already saved, but if we were given a did, that
    // needs to be saved too.
    if ($panelizer->did != $output->did) {
      ctools_include('export');
      $panelizer->did = $output->did;
      // Ensure we don't try to write again:
      if (isset($panelizer->display)) {
        unset($panelizer->display);
      }

      panelizer_save_node_panelizer($panelizer);
    }
    drupal_goto($_GET['q']);
  }

  return $output;
}

/**
 * Page to edit basic settings on a panelized node.
 */
function panelizer_edit_node_content_page($node) {
  $panelizer = panelizer_load_node_panelizer($node);
  if (empty($panelizer)) {
    return MENU_NOT_FOUND;
  }

  $cache = panels_edit_cache_get('panelizer:node:' . $node->nid);

  ctools_include('plugins', 'panels');
  ctools_include('display-edit', 'panels');
  ctools_include('context');
  $renderer = panels_get_renderer_handler('editor', $cache->display);
  $renderer->cache = $cache;

  $output = $renderer->edit();
  if (is_object($output)) {
    // The display was already saved, but if we were given a did, that
    // needs to be saved too.
    if ($panelizer->did != $output->did) {
      ctools_include('export');
      $panelizer->did = $output->did;
      // Ensure we don't try to write again:
      if (isset($panelizer->display)) {
        unset($panelizer->display);
      }

      panelizer_save_node_panelizer($panelizer);
    }
    panels_edit_cache_clear($cache);
    drupal_goto($_GET['q']);
  }

  // Print this with theme('page') so that blocks are disabled while editing a display.
  // This is important because negative margins in common block layouts (i.e, Garland)
  // messes up the drag & drop.
  print theme('page', $output, FALSE);
}

/**
 * Export.inc callback to get a basic display for a new node panelizer.
 */
function panelizer_get_default_display_node() {
  $display = new panels_display;
  $display->layout = 'flexible';
  $display->layout_settings = array();
  $display->panel_settings = array(
    'style_settings' => array(
      'default' => NULL,
      'center' => NULL,
    ),
  );
  $display->cache = array();
  $display->title = '';
  $display->content = array();
  $display->panels = array();
    $pane = new stdClass;
    $pane->pid = 'new-1';
    $pane->panel = 'center';
    $pane->type = 'node_content';
    $pane->subtype = 'node_content';
    $pane->shown = TRUE;
    $pane->access = array();
    $pane->configuration = array(
      'links' => 1,
      'page' => 1,
      'no_extras' => 0,
      'override_title' => 0,
      'override_title_text' => '',
      'identifier' => '',
      'link' => 0,
      'leave_node_title' => 0,
      'context' => 'panelizer',
      'build_mode' => 'full',
    );
    $pane->cache = array();
    $pane->style = array(
      'settings' => NULL,
    );
    $pane->css = array();
    $pane->extras = array();
    $pane->position = 0;
    $display->content['new-1'] = $pane;
    $display->panels['center'][0] = 'new-1';
    $pane = new stdClass;
    $pane->pid = 'new-2';
    $pane->panel = 'center';
    $pane->type = 'node_comments';
    $pane->subtype = 'node_comments';
    $pane->shown = TRUE;
    $pane->access = array();
    $pane->configuration = array(
      'mode' => '4',
      'order' => '2',
      'comments_per_page' => '50',
      'context' => 'panelizer',
      'override_title' => 0,
      'override_title_text' => '',
    );
    $pane->cache = array();
    $pane->style = array(
      'settings' => NULL,
    );
    $pane->css = array();
    $pane->extras = array();
    $pane->position = 1;
    $display->content['new-2'] = $pane;
    $display->panels['center'][1] = 'new-2';
    $pane = new stdClass;
    $pane->pid = 'new-3';
    $pane->panel = 'center';
    $pane->type = 'node_comment_form';
    $pane->subtype = 'node_comment_form';
    $pane->shown = TRUE;
    $pane->access = array();
    $pane->configuration = array(
      'anon_links' => 1,
      'context' => 'panelizer',
      'override_title' => 0,
      'override_title_text' => '',
    );
    $pane->cache = array();
    $pane->style = array(
      'settings' => NULL,
    );
    $pane->css = array();
    $pane->extras = array();
    $pane->position = 2;
    $display->content['new-3'] = $pane;
    $display->panels['center'][2] = 'new-3';
  $display->hide_title = PANELS_TITLE_FIXED;
  $display->title_pane = 'new-1';

  return $display;
}

/**
 * Callback to get the base context for a node panelizer.
 */
function panelizer_get_base_context_node($object = NULL) {
  ctools_include('context');
  if ($object) {
    $context = ctools_context_create('node', $object);
  }
  else {
    $context = ctools_context_create_empty('node');
    // The placeholder is needed to create the form used for the live
    // preview.
    $context->placeholder = array(
      'type' => 'context',
      'conf' => array(
        'name' => 'node',
        'identifier' => t('This node'),
        'keyword' => 'node',
        'context_settings' => array()
      ),
    );
  }

  $context->identifier = t('This node');
  $context->keyword = 'node';
  return array('panelizer' => $context);
}
