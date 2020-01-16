<?php

/**
 * @file
 * briks_f2.features.menu_links.inc
 */

/**
 * Implements hook_menu_default_menu_links().
 */
function briks_f2_menu_default_menu_links() {
  $menu_links = array();

  // Exported menu link: management_:admin/config/briks.
  $menu_links['management_:admin/config/briks'] = array(
    'menu_name' => 'management',
    'link_path' => 'admin/config/briks',
    'router_path' => 'admin/config',
    'link_title' => 'Кирпич',
    'options' => array(
      'attributes' => array(
        'title' => '',
      ),
      'identifier' => 'management_:admin/config/briks',
    ),
    'module' => 'system',
    'hidden' => 0,
    'external' => 0,
    'has_children' => 0,
    'expanded' => 0,
    'weight' => -50,
    'customized' => 1,
    'parent_identifier' => 'management_:admin',
  );

  // Translatables
  // Included for use with string extractors like potx.
  t('Кирпич');

  return $menu_links;
}
