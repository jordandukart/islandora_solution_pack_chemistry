<?php
/**
 * @file
 * Contains all blocks used in the chemical solution pack.
 */

/**
 * Defines an upload file(s) block.
 *
 * @param array $form
 *   An array representing a Drupal form.
 * @param array $form_state
 *   An array containing the Drupal form state.
 * @return array
 *   A renderable array of a Drupal form.
 */
function islandora_chemistry_upload_file_form($form, &$form_state) {
  module_load_include('inc', 'islandora_chemistry', 'includes/utilities');
  form_load_include($form_state, 'inc', 'islandora_chemistry', 'includes/blocks');
  $options = islandora_chemistry_mime_selector();
  $form['upload_file'] = array(
    '#type' => 'managed_file',
    '#default_value' => isset($form_state['values']['upload_file']) ? $form_state['values']['upload_file'] : FALSE,
    '#upload_location' => 'temporary://',
    '#upload_validators' => array(
      'file_validate_extensions' => array(),
    ),
  );
  $form['options'] = $options;
  $form['options']['#description'] = t("If the uploaded file's extension is unknown, this will be used as a fallback.");
  $form['ingest'] = array(
    '#type' => 'submit',
    '#value' => t('Ingest'),
  );
  return $form;
}

function islandora_chemistry_upload_file_form_submit($form, &$form_state) {
  $tuque = islandora_get_tuque_connection();
  $collection = islandora_object_load('islandora:chemistry_collection');
  $policy = new CollectionPolicy($collection['COLLECTION_POLICY']->content);
  $policy_content_models = $policy->getContentModels();
  $content_models = islandora_get_content_models();
  $form_state['content_models'] = $content_models;
  $default_namespace = islandora_get_namespace($policy_content_models['islandora:collectionCModel']['namespace']);
  $object = $tuque->constructObject($default_namespace);
  $ds = $object->constructDatastream('OBJ', 'M');
  $object->ingestDatastream($ds);
  $chem_file = file_load($form_state['values']['file']);
  $chem_path = drupal_realpath($chem_file->uri);
  $ds->label = $chem_file->filename;
  $ds->mimetype = $chem_file->filemime;
  $ds->setContentFromFile($chem_path, FALSE);
}



