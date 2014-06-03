<?php

/**
 * @file
 * Handles the uploading of the chemistry file as part of the ingest process.
 */

/**
 * Defines a file upload form for uploading the chemistry related files.
 *
 * @param array $form
 *   The Drupal form.
 * @param array $form_state
 *   The Drupal form state.
 *
 * @return array
 *   The Drupal form definition.
 */
function islandora_chemistry_upload_form(array $form, array &$form_state) {
  module_load_include('inc', 'islandora_chemistry', 'includes/utilities');
  $upload_size = min((int) ini_get('post_max_size'), (int) ini_get('upload_max_filesize'));
  $form['file'] = array(
    '#title' => t('File'),
    '#type' => 'managed_file',
    '#required' => TRUE,
    '#description' => t(
      'Select file to upload.<br/>Files must be less than <strong>@size MB</strong>.',
      array('@size' => $upload_size)
    ),
    '#default_value' => isset($form_state['values']['file']) ? $form_state['values']['file'] : NULL,
    '#upload_location' => 'temporary://',
    '#upload_validators' => array(
      // Empty extensions so Drupal's default won't be used.
      'file_validate_extensions' => array(),
      // Assume it's specified in MB.
      'file_validate_size' => array($upload_size * 1024 * 1024),
    ),
  );
  $form['type_select_check'] = array(
    '#type' => 'checkbox',
    '#title' => t('Set file type.'),
  );
  $form['type_selector'] = islandora_chemistry_mime_selector();
  $form['type_selector']['#states'] = array(
    'visible' => array(
      ':input[name=type_select_check]' => array('checked' => TRUE),
    ),
  );
  return $form;
}

/**
 * Validation for ingest upload form.
 *
 * @param array $form
 *   The Drupal form.
 * @param array $form_state
 *   The Drupal form state.
 */
function islandora_chemistry_upload_form_validate(array $form, array &$form_state) {
  $values = &$form_state['values'];
  $chem_file = file_load($values['file']);
  $chem_file_mime = &$chem_file->filemime;
  if (!$values['type_select_check']) {
    module_load_include('inc', 'islandora_chemistry', 'includes/utilities');
    if (!in_array($chem_file_mime, array_keys(islandora_chemistry_get_chemistry_mimes()))) {
      form_set_error('type_select_check', t("The file's type couldn't be parsed please select a type."));
    }
  }
}

/**
 * Adds the uploaded file into the object's datastream.
 *
 * @param array $form
 *   The Drupal form.
 * @param array $form_state
 *   The Drupal form state.
 */
function islandora_chemistry_upload_form_submit(array $form, array &$form_state) {
  module_load_include('inc', 'islandora', 'includes/ingest.form');
  $values = &$form_state['values'];
  $object = islandora_ingest_form_get_object($form_state);
  if (empty($object['OBJ'])) {
    $ds = $object->constructDatastream('OBJ', 'M');
    $object->ingestDatastream($ds);
  }
  else {
    $ds = $object['OBJ'];
  }

  $chem_file = file_load($values['file']);
  $chem_path = drupal_realpath($chem_file->uri);
  $ds->label = $chem_file->filename;
  $ds->mimetype = $values['type_select_check'] ? $values['type_selector'] : $chem_file->filemime;
  $ds->setContentFromFile($chem_path, FALSE);
}
