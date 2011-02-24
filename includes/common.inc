<?php
// $Id: $

/**
 * @file
 * Contains common forms and routines that different object types use.
 *
 * The panelizer system has several different places that panels can be
 * edited. While they are all subtly different, they have a lot in common.
 */

/**
 * Form to configure basic panelizer settings.
 */
function panelizer_settings_form(&$form_state) {
  $panelizer = $form_state['panelizer'];

  $form['no_blocks'] = array(
    '#type' => 'checkbox',
    '#default_value' => $panelizer->no_blocks,
    '#title' => t('Disable Drupal blocks/regions'),
    '#description' => t('Check this to have the panel disable sidebars displayed in the theme. Note that some themes support this setting better than others. If in doubt, try with stock themes to see.'),
  );

  ctools_include('plugins', 'panels');
  $pipelines = panels_get_renderer_pipelines();

  // If there are no pipelines, that probably means we're operating in
  // legacy mode.
  if (empty($pipelines)) {
    // We retain the original pipeline so we don't wreck things by installing
    // old modules.
    $form['pipeline'] = array(
      '#type' => 'value',
      '#value' => $panelizer->pipeline,
    );
  }
  else {
    $options = array();
    foreach ($pipelines as $name => $pipeline) {
      $options[$name] = check_plain($pipeline->admin_title) . '<div class="description">' . check_plain($pipeline->admin_description) . '</div>';
    }

    $form['pipeline'] = array(
      '#type' => 'radios',
      '#options' => $options,
      '#title' => t('Renderer'),
      '#default_value' => $panelizer->pipeline,
    );
  }

  $form['css_id'] = array(
    '#type' => 'textfield',
    '#size' => 35,
    '#default_value' => $panelizer->css_id,
    '#title' => t('CSS ID'),
    '#description' => t('The CSS ID to apply to this panel'),
  );

  $form['css'] = array(
    '#type' => 'textarea',
    '#title' => t('CSS code'),
    '#description' => t('Enter well-formed CSS code here; this code will be embedded into the panel, and should only be used for minor adjustments; it is usually better to try to put CSS for the panel into the theme if possible. This CSS will be filtered for safety so some CSS may not work.'),
    '#default_value' => $panelizer->css,
  );

  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Save'),
  );

  return $form;
}

/**
 * Submit callback
 */
function panelizer_settings_form_submit(&$form, &$form_state) {
  $panelizer = &$form_state['panelizer'];
  $panelizer->no_blocks = $form_state['values']['no_blocks'];
  $panelizer->css = $form_state['values']['css'];
  $panelizer->css_id = $form_state['values']['css_id'];
  $panelizer->pipeline = $form_state['values']['pipeline'];
}

/**
 * Form to edit contexts that go with a panelizer panel.
 */
function panelizer_default_context_form(&$form_state) {
  $form = array();
  ctools_include('context-admin');
  ctools_context_admin_includes();

  $panelizer = &$form_state['panelizer'];
  $panelizer->base_contexts = panelizer_get_base_context($form_state['panelizer id']);

  if (!empty($panelizer->cached)) {
    $form['markup'] = array(
      '#prefix' => '<div class="messages warning">',
      '#value' => t('This form contains unsaved changes that will not be stored until the Save button is clicked.'),
      '#suffix' => '</div>',
    );
  }

  $form['right'] = array(
    '#prefix' => '<div class="clear-block"><div class="right-container">',
    '#suffix' => '</div>',
  );

  $form['left'] = array(
    '#prefix' => '<div class="left-container">',
    '#suffix' => '</div></div>',
  );

  $module = 'panelizer-' . $form_state['panelizer type'];
  ctools_context_add_context_form($module, $form, $form_state, $form['right']['contexts_table'], $panelizer, $form_state['cache key']);
  ctools_context_add_relationship_form($module, $form, $form_state, $form['right']['relationships_table'], $panelizer, $form_state['cache key']);

  // Set an additional description if CCK and Token are enabled, to notify of unlisted keywords
  if (module_exists('content') && module_exists('token')) {
    $description = t('Note that CCK fields may be used as keywords using patterns like <em>%node:field_name-formatted</em>.');
  }
  elseif (!module_exists('token')) {
    $description = t('More keywords will be available if you install the Token module, see http://drupal.org/project/token.');
  }

  $form['left']['summary'] = array(
    '#prefix' => '<div class="page-manager-contexts">',
    '#suffix' => '</div>',
    '#value' => theme('ctools_context_list', $panelizer, t('Summary of contexts'), $description),
  );

  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Save'),
    '#write' => TRUE,
  );

  $form['cancel'] = array(
    '#type' => 'submit',
    '#value' => t('Cancel'),
  );

  return $form;
}

