<?php

/**
 * @file
 * unload_bricks.features.inc
 */

/**
 * Implements hook_node_info().
 */
function unload_bricks_node_info() {
  $items = array(
    'brick' => array(
      'name' => t('Кирпич'),
      'base' => 'node_content',
      'description' => '',
      'has_title' => '1',
      'title_label' => t('Title'),
      'help' => '',
    ),
  );
  drupal_alter('node_info', $items);
  return $items;
}
