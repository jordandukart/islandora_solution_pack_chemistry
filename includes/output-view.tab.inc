<?php

/**
 * @file
 * Function(s) associated with the "Output" view.
 */

/**
 * Generate the output view.
 *
 * @param AbstractObject $object
 *   The object for which to generate the view.
 *
 * @return array
 *   A renderable array.
 */
function islandora_chemistry_output_view(AbstractObject $object) {
  module_load_include('inc', 'islandora_chemistry', 'includes/utilities');
  islandora_chemistry_set_title($object);

  return array(
    array(
      '#type' => 'markup',
      '#theme' => 'islandora_datastream_download_link',
      '#datastream' => $object['OBJ'],
    ),
    array(
      '#type' => 'markup',
      '#prefix' => '<pre>',
      '#suffix' => '</pre>',
      '#markup' => $object['OBJ']->content,
    ),
  );
}
