<?php

/**
 * @file
 * Theme hooks.
 */

/**
 * Prepares variables for islandora_chemistry templates.
 *
 * Default template: theme/islandora-chemistry.tpl.php
 *
 * @param array $variables
 *   An associative array containing:
 *   - islandora_object: An AbstractObject for which we are to render a
 *     display.
 *   - elements: An array of renderable elements we populate, to include:
 *     - image: An image to output.
 *     - viewer: A ChemDoodle 3D view.
 *     - metadata: A metadata display.
 */
function template_preprocess_islandora_chemistry(&$variables) {
  if (islandora_datastream_access(ISLANDORA_VIEW_OBJECTS, $variables['islandora_object']['MEDIUM'])) {
    $variables['elements']['image'] = array(
      '#type' => 'markup',
      '#theme' => 'image',
      '#path' => "islandora/object/{$variables['islandora_object']->id}/datastream/MEDIUM/view",
    );
  }
  else {
    $variables['elements']['image'] = array();
  }

  if (islandora_datastream_access(ISLANDORA_VIEW_OBJECTS, $variables['islandora_object']['MOL'])) {
    $chem_doodle_path = libraries_get_path('ChemDoodleWeb');
    $chem_path = drupal_get_path('module', 'islandora_chemistry');
    $element_id = drupal_html_id('islandora_chemistry_3d_view');
    $variables['elements']['viewer'] = array(
      '#type' => 'markup',
      '#attached' => array(
        'js' => array(
          array(
            'type' => 'setting',
            'data' => array(
              'islandora_chemistry_3d_view' => array(
                'mol_file' => $variables['islandora_object']['MOL']->content,
                'element_id' => $element_id,
                'element_selector' => "#$element_id",
                'width' => 250,
                'height' => 250,
              ),
            ),
          ),
          "$chem_doodle_path/install/ChemDoodleWeb.js",
          "$chem_path/js/3d-viewer.js",
        ),
        'css' => array(
          "$chem_doodle_path/install/ChemDoodleWeb.css",
        ),
      ),
      '#markup' => "<canvas id='$element_id'/>",
    );
  }
  else {
    $variables['elements']['viewer'] = array();
  }

  module_load_include('inc', 'islandora', 'includes/metadata');
  $module_path = drupal_get_path('module', 'islandora_chemistry');
  $variables['elements']['metadata'] = array(
    '#type' => 'markup',
    '#attached' => array(
      'js' => array(
        'misc/form.js',
        'misc/collapse.js',
      ),
      'css' => array(
        "$module_path/css/islandora_chemistry.theme.css",
      ),
    ),
    '#markup' => islandora_retrieve_metadata_markup($variables['islandora_object'], TRUE),
  );
}

/**
 * Process variables for islandora_chemistry templates.
 *
 * Default template: theme/islandora-chemistry.tpl.php
 *
 * @see template_preprocess_islandora_chemistry()
 *
 * @param array $variables
 *   An associative array containing:
 *   - elements: An array of elements to render, including:
 *     - image: An image to output.
 *     - viewer: A 3D view of the molecule.
 *     - metadata: Metadata to output about the molecule.
 *     For each item in "elements", we will create a value with the same key in
 *     the $variables array with the rendered element.
 */
function template_process_islandora_chemistry(&$variables) {
  foreach ($variables['elements'] as $key => $element) {
    $variables[$key] = drupal_render($element);
  }
}
