<?php

/**
 * @file
 * Function(s) associated wtih rendering the 3D molecule view tab.
 */

/**
 * Setup 3D molecule view.
 *
 * @param AbstractObject $object
 *   An object to render.
 *
 * @return array
 *   A renderable array setting up the display.
 */
function islandora_chemistry_3d_view(AbstractObject $object) {
  $chem_doodle_path = libraries_get_path('ChemDoodleWeb');
  $chem_path = drupal_get_path('module', 'islandora_chemistry');
  $element_id = drupal_html_id('islandora_chemistry_3d_view');
  return array(
    '#type' => 'markup',
    '#attached' => array(
      'js' => array(
        array(
          'type' => 'setting',
          'data' => array(
            'islandora_chemistry_3d_view' => array(
              'mol_file' => $object['MOL']->content,
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
