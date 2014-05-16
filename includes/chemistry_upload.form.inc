<?php

/**
 * @file
 * Handles the uploading of the chemistry file as part of the ingest process.
 */

/**
 * Defines a file upload form for uploading the chemistry related files.
 *
 * @param array $form
 *   The drupal form.
 * @param array $form_state
 *   The drupal form state.
 *
 * @return array
 *   The drupal form definition.
 */
function islandora_chemistry_upload_form(array $form, array &$form_state) {
  $upload_size = min((int) ini_get('post_max_size'), (int) ini_get('upload_max_filesize'));
  $extensions = array('mol xyz pdb cdx c3d chm cif cml imp gam out cub gau gal jdx dx mop in txt');
  $tn_extensions = array('jpg jpeg png');
  $form = array();
  $form['file'] = array(
    '#title' => t('File'),
    '#type' => 'managed_file',
    '#required' => TRUE,
    '#description' => t('Select file to upload.<br/>Files must be less than <b>@size MB.</b><br/>Allowed file types: <b>@ext.</b>', array('@size' => $upload_size, '@ext' => $extensions[0])),
    '#default_value' => isset($form_state['values']['files']) ? $form_state['values']['files'] : NULL,
    '#upload_location' => 'temporary://',
    '#upload_validators' => array(
      'file_validate_extensions' => $extensions,
      // Assume it's specified in MB.
      'file_validate_size' => array($upload_size * 1024 * 1024),
    ),
  );
  $form['tn'] = array(
    '#title' => t('Thumbnail for object'),
    '#type' => 'managed_file',
    '#required' => FALSE,
    '#description' => t('Select text file to upload.<br/>Files must be less than <b>@size MB.</b><br/>Allowed file types: <b>@ext.</b><br />This file is optional.', array('@size' => $upload_size, '@ext' => $tn_extensions[0])),
    '#default_value' => isset($form_state['values']['files']) ? $form_state['values']['files'] : NULL,
    '#upload_location' => 'temporary://',
    '#upload_validators' => array(
      'file_validate_extensions' => $tn_extensions,
      // Assume it's specified in MB.
      'file_validate_size' => array($upload_size * 1024 * 1024),
    ),
    '#states' => array(
      'visible' => array(
        ':input[name="islandora_chemistry_tn_upload"]' => array('checked' => TRUE),
      ),
    ),
  );

  return $form;
}

/**
 * Addes the uploaded file into the ingestable objects datastream.
 *
 * @param array $form
 *   The drupal form.
 * @param array $form_state
 *   The drupal form state.
 */
function islandora_chemistry_upload_form_submit(array $form, array &$form_state) {
  module_load_include('inc', 'islandora', 'includes/mime_detect');
  $object = $form_state['islandora']['objects'][0];
  if (empty($object['OBJ'])) {
    $ds = $object->constructDatastream('OBJ', 'M');
    $object->ingestDatastream($ds);
  }
  else {
    $ds = $object['OBJ'];
  }

  $chem_file = file_load($form_state['values']['file']);
  $chem_path = drupal_realpath($chem_file->uri);
  $ds->label = $chem_file->filename;
  $ds->mimetype = $object['OBJ']->mimeType;
  $ds->setContentFromFile($chem_path, FALSE);
  $tn_file = file_load($form_state['values']['tn']);
  if (isset($tn_file->uri)) {
    if ($form_state['values']['tn']) {
      islandora_scale_thumbnail($tn_file, 200, 200);
    }
    if (empty($object['TN'])) {
      $tn = $object->constructDatastream('TN', 'M');
      $object->ingestDatastream($tn);
    }
    else {
      $tn = $object['TN'];
    }
    $tn->setContentFromFile(drupal_realpath($tn_file->uri), FALSE);
    if ($tn->label != $tn_file->filename) {
      $tn->label = $tn_file->filename;
    }
    if ($tn->mimetype != $tn_file->filemime) {
      $tn->mimetype = $tn_file->filemime;
    }
  }
}
