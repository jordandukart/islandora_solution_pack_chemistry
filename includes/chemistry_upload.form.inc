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
    '#default_value' => isset($form_state['values']['file']) ? $form_state['values']['file'] : NULL,
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
    '#default_value' => isset($form_state['values']['tn']) ? $form_state['values']['tn'] : NULL,
    '#upload_location' => 'temporary://',
    '#upload_validators' => array(
      'file_validate_extensions' => $tn_extensions,
      // Assume it's specified in MB.
      'file_validate_size' => array($upload_size * 1024 * 1024),
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
  module_load_include('inc', 'islandora', 'includes/ingest.form');
  $object = islandora_ingest_form_get_object($form_state);
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

function islandora_chemistry_simple_ingest($file_name, $separate = TRUE) {
  global $user;

  $file_name_array = array();
  $file_name_array['OBJ'] = $file_name;

  $title =  basename($file_name);

  $transform = "/mods_to_dc.xsl";//drupal_get_path('module', 'islandora_content_model_forms') . "/transforms/mods_to_dc.xsl";
  $content_model_pid = 'islandora:sp_chem_CM';
  $collection_pid = 'quantumchem:sp_chem_calculations';
  $dsid = 'MODS';
  $pid = Fedora_Item::get_next_PID_in_namespace('islandora');
  $username = $user->name;
  $xml = "<mods xmlns=\"http://www.loc.gov/mods/v3\">
  <titleInfo>
    <title>$title</title>
  </titleInfo>
  <name type=\"personal\">
    <namePart>$username</namePart>
    <role>
      <roleTerm authority=\"marcrelator\" type=\"text\">Author</roleTerm>
    </role>
  </name>
</mods>";

  $document = new DOMDocument();
  $document->loadXML($xml);
  $document2 = $document->saveXML();
  $document3 = new DOMDocument();
  $document3->loadXML($document2);

  $collection_policy = CollectionPolicy::loadFromCollection($collection_pid);
  if ($collection_policy !== FALSE) {
    $relationship = $collection_policy->getRelationship();
  }

  $ingest = new IslandoraChem($pid);
  $mol = $ingest->chem2mol(NULL, 'MOL', $file_name, 'mol');
  if ($mol != 1) {
    drupal_set_message(t("Creating an archival version of $file_name failed!"), 'error');
    return;
  }
  $file_name_array['MOL'] = $_SESSION['fedora_ingest_files']['MOL'];
  $cml = $ingest->chem2cml(NULL, 'CML', $file_name, 'xml', $separate);
  if ($cml != 1) {
    drupal_set_message(t("Extracting metadata from $file_name failed!"), 'error');
    return;
  }
  $file_name_array['CML'] = $_SESSION['fedora_ingest_files']['CML'];

  $jpg = $ingest->chem2jpg(array('width' => '100', 'height' => '100'), 'TN', $file_name, 'jpg');
  if ($jpg != 1) {
    drupal_set_message(t("Creating the thumbnail for $file_name failed!"), 'error');
    return;
  }
  $file_name_array['TN'] = $_SESSION['fedora_ingest_files']['TN'];
  $medium = $ingest->chem2jpg(array('width' => '250', 'height' => '250'), 'MEDIUM', $file_name, 'jpg');
  if ($medium != 1) {
    drupal_set_message(t("Creating an image of $file_name failed!"), 'error');
    return;
  }

  $file_name_array['MEDIUM'] = $_SESSION['fedora_ingest_files']['MEDIUM'];
  $object = new FOXML($title, $pid, $dsid, $content_model_pid, $collection_pid, $relationship, $file_name_array, $document3, $transform);
  $submit = $object->ingest();
  if (!$submit) {
    drupal_set_message(t("An error occurred when saving the file!"), 'error');
  }
}
