<?php
// $Id: $

/**
 * @file
 * Contains administrative forms and settings.
 */

/**
 * Primary settings page.
 *
 * This settings page allows the administrator to select which node types
 * can be panelized, whether they have a default, and provides links to
 * edit those defaults.
 */
function panelizer_settings_page_form(&$form_state) {
  ctools_include('dependent');
  $form = array();
  $form['warnings'] = array();
  $task = page_manager_get_task('node_view');
  if (!empty($task->disabled)) {
    $form['warning']['task'] = array(
      '#prefix' => '<div class="messages warning">',
      '#value' => t('The node template page is currently not enabled in page manager. You must enable this for Panelizer to be able to panelize nodes.'),
      '#suffix' => '</div>',
    );
  }

  $handler = page_manager_load_task_handler($task, '', 'node_view_panelizer');
  if (!empty($handler->disabled)) {
    $form['warning']['handler'] = array(
      '#prefix' => '<div class="messages warning">',
      '#value' => t('The panelizer variant on the node template page is currently not enabled in page manager. You must enable this for Panelizer to be able to panelize nodes.'),
      '#suffix' => '</div>',
    );
  }

  $form['types'] = array('#tree' => TRUE);

  // Add 'node' settings
  $form['types']['node'] = array(
    '#theme' => 'panelizer_node_settings_page_form',
  );

  $types = node_get_types('names');
  drupal_alter('panelizer_default_types', $types, 'node');

  $settings = variable_get('panelizer_defaults', array());

  foreach ($types as $type => $title) {
    $base_id = str_replace(array('][', '_', ' '), '-', 'edit-types-node-' . $type);

    $form['types']['node'][$type]['title'] = array(
      '#value' => $title,
      // '#markup' => $title, // for d7 port
    );

    $form['types']['node'][$type]['status'] = array(
      '#type' => 'checkbox',
      '#default_value' => !empty($settings['node'][$type]['status']),
    );

    $form['types']['node'][$type]['default'] = array(
      '#type' => 'checkbox',
      '#default_value' => !empty($settings['node'][$type]['default']),
      '#process' => array('ctools_dependent_process'),
      '#dependency' => array($base_id . '-status' => array(TRUE)),
    );

    $form['types']['node'][$type]['choice'] = array(
      '#type' => 'checkbox',
      '#default_value' => !empty($settings['node'][$type]['choice']),
      '#process' => array('ctools_dependent_process'),
      '#dependency' => array($base_id . '-status' => array(TRUE)),
      '#access' => FALSE, // Allowing a choice is curently disabled
    );

    $base_url = 'admin/settings/panelizer/node/' . $type;

    $form['types']['node'][$type]['links'] = array(
      '#prefix' => '<div class="container-inline">',
      '#suffix' => '</div>',
    );

    if (!empty($settings['node'][$type]['status'])) {
      // add links
      $links = array(
        'settings' => array(
          'title' => t('allowed content'),
          'href' => $base_url . '/allowed',
        ),
      );

      $form['types']['node'][$type]['links']['basic'] = array(
        '#type' => 'item',
        '#input' => TRUE, // necessary to fake the #process
        '#value' => theme('links', $links, array('class' => 'links inline')),
        '#id' => $base_id . '-links-basic',
        '#process' => array('ctools_dependent_process'),
        '#dependency' => array($base_id . '-status' => array(TRUE)),
      );
    }

    if (!empty($settings['node'][$type]['default'])) {
      // add links
      $links = array(
        'settings' => array(
          'title' => t('settings'),
          'href' => $base_url . '/settings',
        ),
        'context' => array(
          'title' => t('context'),
          'href' => $base_url . '/context',
        ),
        'layout' => array(
          'title' => t('layout'),
          'href' => $base_url . '/layout',
        ),
        'content' => array(
          'title' => t('content'),
          'href' => $base_url . '/content',
        ),
      );

      $form['types']['node'][$type]['links']['default'] = array(
        '#type' => 'item',
        '#input' => TRUE, // necessary to fake the #process
        '#value' => theme('links', $links, array('class' => 'links inline')),
        '#id' => $base_id . '-links-default',
        '#process' => array('ctools_dependent_process'),
        '#dependency_count' => 2,
        '#dependency' => array(
          $base_id . '-default' => array(TRUE),
          $base_id . '-choice' => array(FALSE),
        ),
      );

      $links = array(
        'list' => array(
          'title' => t('list'),
          'href' => $base_url . '/list',
        ),
      );

      $form['types']['node'][$type]['links']['default2'] = array(
        '#type' => 'item',
        '#input' => TRUE, // necessary to fake the #process
        '#value' => theme('links', $links, array('class' => 'links inline')),
        '#id' => $base_id . '-links-default2',
        '#process' => array('ctools_dependent_process'),
        '#dependency_count' => 2,
        '#dependency' => array(
          $base_id . '-default' => array(TRUE),
          $base_id . '-choice' => array(TRUE),
        ),
      );
    }
  }

  // Add 'user' settings


  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Save'),
  );

  return $form;
}

/**
 * Format the output of the main settings form.
 *
 * We want our checkboxes to show up in a table.
 */
function theme_panelizer_node_settings_page_form($element) {
  $output = '';

  // Render the 'node' table
  $header = array(
    t('Node type'),
    t('Panelize'),
    t('Provide default panel'),
    t('Operations'),
  );

  $rows = array();
  foreach (element_children($element) as $type) {
    $rows[] = array(
      drupal_render($element[$type]['title']),
      drupal_render($element[$type]['status']),
      drupal_render($element[$type]['default']),
      drupal_render($element[$type]['links']),
    );
  }

  $output .= theme('table', $header, $rows);

  // Render the 'user' table

  // Render everything else
  $output .= drupal_render($element);
  return $output;
}

/**
 * Submit callback for the main panelizer settings form.
 */
function panelizer_settings_page_form_submit($form, &$form_state) {
  $values = $form_state['values']['types'];

  // Since we used #type => 'item' to fake the #process, there's a bad
  // value here. Kill it.
  foreach ($values as $type => $info) {
    unset($values[$type]['links']);
  }

  variable_set('panelizer_defaults', $values);
}

/**
 * Page to configure what content is available for a given node type.
 */
function panelizer_allowed_content_page($type, $key) {
  if (!panelizer_is_panelized($type, $key)) {
    return MENU_NOT_FOUND;
  }

  // Drupal kills our breadcrumb. Put it back.
  ctools_include('menu');
  ctools_menu_set_trail_parent('admin/settings/panelizer');

  ctools_include('common', 'panels');
  return drupal_get_form('panels_common_settings', 'panelizer_' . $type . ':' . $key);
}

/**
 * Page to list available panelizer defaults for a given type and key combo.
 */
function panelizer_default_list_page($type, $key) {
  return 'placeholder';
}

/**
 * Page to configure basic settings for a panelizer default.
 */
function panelizer_default_settings_page($type, $key, $name) {
  $panelizer = panelizer_load_default($type, $key, $name);
  if (empty($panelizer)) {
    return MENU_NOT_FOUND;
  }

  // Drupal kills our breadcrumb. Put it back.
  ctools_include('menu');
  ctools_menu_set_trail_parent('admin/settings/panelizer');

  $form_state = array(
    'panelizer' => &$panelizer,
    'no_redirect' => TRUE,
  );

  ctools_include('form');
  ctools_include('common', 'panelizer');
  $output = ctools_build_form('panelizer_settings_form', $form_state);
  if (!empty($form_state['executed'])) {
    drupal_set_message(t('The settings have been updated.'));
    ctools_include('export');
    ctools_export_crud_save('panelizer_defaults', $panelizer);
    drupal_goto($_GET['q']);
  }

  return $output;
}


/**
 * Page to configure what content is available for a given node type.
 */
function panelizer_default_context_page($type, $key, $name) {
  $cache_key = $type . ':' . $key . ':' . $name;
  $panelizer = panelizer_context_cache_get('default', $cache_key);
  if (empty($panelizer)) {
    return MENU_NOT_FOUND;
  }

  // Drupal kills our breadcrumb. Put it back.
  ctools_include('menu');
  ctools_menu_set_trail_parent('admin/settings/panelizer');

  $form_state = array(
    'panelizer' => &$panelizer,
    'panelizer type' => 'default',
    'panelizer id' => $type,
    'cache key' => $cache_key,
    'no_redirect' => TRUE,
  );

  ctools_include('form');
  ctools_include('common', 'panelizer');
  $output = ctools_build_form('panelizer_default_context_form', $form_state);
  if (!empty($form_state['executed'])) {
    if (!empty($form_state['clicked_button']['#write'])) {
      drupal_set_message(t('The settings have been updated.'));
      ctools_include('export');
      ctools_export_crud_save('panelizer_defaults', $panelizer);
    }
    else {
      drupal_set_message(t('Changes have been discarded.'));
    }

    panelizer_context_cache_clear('default', $cache_key);
    drupal_goto($_GET['q']);
  }

  return $output;
}

/**
 * Pass through to the panels layout editor.
 */
function panelizer_default_layout_page($type, $key, $name) {
  $panelizer = panelizer_load_default($type, $key, $name);
  if (empty($panelizer)) {
    return MENU_NOT_FOUND;
  }

  // Drupal kills our breadcrumb. Put it back.
  ctools_include('menu');
  ctools_menu_set_trail_parent('admin/settings/panelizer');

  $display = panelizer_load_display($panelizer);

  ctools_include('context');
  $display->context = panelizer_get_contexts($panelizer);
  $output = panels_edit_layout($display, t('Save'), '', 'panelizer_' . $type . ':' . $key);
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

      ctools_export_crud_save('panelizer_defaults', $panelizer);
    }
    drupal_goto($_GET['q']);
  }

  return $output;
}

/**
 * Pass through to the panels content editor.
 */
function panelizer_default_content_page($type, $key, $name) {
  $panelizer = panelizer_load_default($type, $key, $name);
  if (empty($panelizer)) {
    return MENU_NOT_FOUND;
  }

  // Drupal kills our breadcrumb. Put it back.
  ctools_include('menu');
  ctools_menu_set_trail_parent('admin/settings/panelizer');

  $cache = panels_edit_cache_get('panelizer:default:' . $type . ':' . $key . ':' . $name);

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

      ctools_export_crud_save('panelizer_defaults', $panelizer);
    }
    panels_edit_cache_clear($cache);
    drupal_goto($_GET['q']);
  }

  // Print this with theme('page') so that blocks are disabled while editing a display.
  // This is important because negative margins in common block layouts (i.e, Garland)
  // messes up the drag & drop.
  print theme('page', $output, FALSE);
}
