<?php
// $Id: $

/**
 * @file
 *
 * This is the task handler plugin to handle node_view.
 */

// Plugin definition
$plugin = array(
  // is a 'context' handler type, meaning it supports the API of the
  // context handlers provided by ctools context plugins.
  'handler type' => 'context',
  // may NOT be added up front.
  'visible' => FALSE,

  // Administrative fields.
  'title' => t('Panelizer'),
  'admin summary' =>'panelizer_panelizer_task_admin_summary',
  'operations' => array(),

  // Callback to render the data.
  'render' => 'panelizer_panelizer_task_render',
);

function panelizer_panelizer_task_admin_summary($handler, $task, $subtask, $page, $show_title = TRUE) {
  $output = '';

  $output .= '<div class="clear-block">';
  if ($show_title) {
    // Get the operations
    $operations = page_manager_get_operations($page);

    // Get operations for just this handler.
    $operations = $operations['handlers']['children'][$handler->name]['children']['actions']['children'];
    $args = array('handlers', $handler->name, 'actions');
    $rendered_operations = page_manager_render_operations($page, $operations, array(), array('class' => 'actions'), 'actions', $args);

    $output .= '<div class="handler-title clear-block">';
    $output .= '<div class="actions handler-actions">' . $rendered_operations['actions'] . '</div>';
    $output .= '<span class="title-label">' . t('Panelizer') . '</span>';
    $output .= '</div>';
  }

  $access = t('This variant will be selected if the node being viewed is panelized. This variant must be enabled and selected for panelizer to work!');

  $rows[] = array(
    array('class' => t('page-summary-label'), 'data' => t('Selection rule')),
    array('class' => t('page-summary-data'), 'data' => $access),
    array('class' => t('page-summary-operation'), ''),
  );

  $output .= theme('table', array(), $rows, array('class' => 'page-manager-handler-summary'));
  $output .= '</div>';

  return $output;
}

/**
 * Render a node that has been panelized.
 */
function panelizer_panelizer_task_render($handler, $base_contexts, $args, $test = TRUE) {
  // The node this is for should always be the first context available. If
  // this is not a node, bail. This should allow us to be used on things
  // that aren't node_view if, for some weird reason, people want to.
  if (empty($base_contexts)) {
    return;
  }

  $context = reset($base_contexts);
  if ($context->type != 'node' || empty($context->data)) {
    return;
  }

  // Load the panelizer.
  $node = &$context->data;
  ctools_include('node', 'panelizer');
  $panelizer = panelizer_load_node_panelizer($node);
  if (empty($panelizer)) {
    return;
  }

  // Load the display
  $display = panelizer_load_display($panelizer);
  if (empty($display)) {
    return;
  }

  // This converts the $node we extracted from $base_contexts into a new
  // context for the panelized node, plus whatever other contexts were
  // configured for it. If for some reason there were other contexts in
  // the $base_contexts, they are not available.
  $display->context = panelizer_get_contexts($panelizer, $node);
  $display->args = $args;
  $display->css_id = $panelizer->css_id;
  $task_name = page_manager_make_task_name($handler->task, $handler->subtask);

  // This means the IPE will use our cache which means it will get appropriate
  // allowed content should it be selected.
  $display->cache_key = 'panelizer:node:' . $node->nid;

  // Check to see if there is any CSS.
  if (!empty($panelizer->css)) {
    ctools_include('css');
    $filename = ctools_css_retrieve($display->cache_key);
    if (!$filename) {
      $filename = ctools_css_store($display->cache_key, $panelizer->css);
    }
    ctools_css_add_css($filename);
  }

  // We think this is handled as a page, so set the current page display.
  panels_get_current_page_display($display);

  ctools_include('plugins', 'panels');
  $renderer = panels_get_renderer($panelizer->pipeline, $display);

  $info = array(
    'content' => panels_render_display($display, $renderer),
    'no_blocks' => !empty($panelizer->no_blocks),
  );

  return $info;
}
