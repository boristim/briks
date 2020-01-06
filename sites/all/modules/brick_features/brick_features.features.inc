<?php

/**
 * @file
 * brick_features.features.inc
 */

/**
 * Implements hook_views_api().
 */
function brick_features_views_api($module = NULL, $api = NULL) {
  return array("api" => "3.0");
}

/**
 * Implements hook_node_info().
 */
function brick_features_node_info() {
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
