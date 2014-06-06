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
 *
 * @return array
 *   A renderable array of a Drupal form.
 */
function islandora_chemistry_upload_file_form($form, &$form_state) {
  form_load_include($form_state, 'inc', 'islandora_chemistry', 'includes/utilities');
  form_load_include($form_state, 'inc', 'islandora_chemistry', 'includes/blocks');
  $upload_size = min((int) ini_get('post_max_size'), (int) ini_get('upload_max_filesize'));
  $options = islandora_chemistry_mime_selector();
  $form['upload_file'] = array(
    '#title' => t('Upload A File'),
    '#type' => 'managed_file',
    '#required' => TRUE,
    '#description' => t(
      'Select file to upload.<br/>Files must be less than <strong>@size MB</strong>.',
      array('@size' => $upload_size)
    ),
    '#default_value' => isset($form_state['values']['upload_file']) ? $form_state['values']['upload_file'] : FALSE,
    '#upload_location' => 'temporary://',
    '#upload_validators' => array(
      'file_validate_extensions' => array(),
      'file_validate_size' => array($upload_size * 1024 * 1024),
    ),
  );
  $form['type_select_check'] = array(
    '#type' => 'checkbox',
    '#title' => t('Set file type.'),
  );
  $form['options'] = $options;
  $form['options']['#description'] = t("If the uploaded file's extension is unknown, this will be used as a fallback. In the case of a ZIP this MIME type will be used as the MIME type for all objects in the ZIP.");
  $form['options']['#states'] = array(
    'visible' => array(
      ':input[name="type_select_check"]' => array('checked' => TRUE),
    ),
  );
  $form['ingest'] = array(
    '#type' => 'submit',
    '#value' => t('Ingest'),
  );
  return $form;
}

/**
 * Validation for the chemistry upload block.
 *
 * @param array $form
 *   An array representing a Drupal form.
 * @param array $form_state
 *   An array containing the Drupal form state.
 */
function islandora_chemistry_upload_file_form_validate($form, &$form_state) {
  $values = &$form_state['values'];
  $chem_file = file_load($values['upload_file']);
  if ($chem_file) {
    $chem_file_mime = $chem_file->filemime;
  }
  if ($chem_file && !$values['type_select_check']) {
    module_load_include('inc', 'islandora_chemistry', 'includes/utilities');
    if ($chem_file_mime == 'application/zip') {
      form_error($form['type_select_check'], t('A type is required when ingesting a ZIP.'));
    }
    elseif (!in_array($chem_file_mime, array_keys(islandora_chemistry_get_chemistry_mimes()))) {
      form_error($form['type_select_check'], t("The file's type couldn't be parsed please select a type."));
    }
  }
}

/**
 * Submit handler for the chemistry upload block.
 *
 * @param array $form
 *   An array representing a Drupal form.
 * @param array $form_state
 *   An array containing the Drupal form state.
 */
function islandora_chemistry_upload_file_form_submit($form, &$form_state) {
  module_load_include('inc', 'islandora', 'includes/utilities');
  $tuque = islandora_get_tuque_connection();
  $collection = islandora_object_load('islandora:chemistry_collection');
  $policy = new CollectionPolicy($collection['COLLECTION_POLICY']->content);
  $policy_content_models = $policy->getContentModels();
  $default_namespace = islandora_get_namespace($policy_content_models['islandora:sp_chem_CM']['namespace']);
  $file = file_load($form_state['values']['upload_file']);
  if ($file->filemime == 'application/zip') {
    $params = array(
      'file' => file_load($form_state['values']['upload_file']),
      'pid_namespace' => $default_namespace,
      'content_model' => array('islandora:sp_chem_CM'),
      'processed_objects' => array(),
      'file_mime' => $form_state['values']['options'],
    );
    $preprocessor = new IslandoraChemistryZipBatchImporter(
      $tuque,
      IslandoraChemistryZipBatchImporter::getBatchInfo($params),
      'islandora:chemistry_collection'
    );

    islandora_batch_ingest(array(
      'preprocessor' => $preprocessor,
    ));
  }
  else {
    // @TODO: Set label via derivatives?
    $object = $tuque->repository->constructObject($default_namespace);
    $object->relationships->add(FEDORA_RELS_EXT_URI, 'isMemberOfCollection', 'islandora:chemistry_collection');
    $object->models = array('islandora:sp_chem_CM');
    $ds = $object->constructDatastream('OBJ', 'M');
    $object->ingestDatastream($ds);
    $ds->label = $file->filename;
    $ds->mimetype = $form_state['values']['type_select_check'] ? $form_state['values']['options'] : $file->filemime;
    $ds->setContentFromFile($file->uri, FALSE);
    $tuque->repository->ingestObject($object);
  }
}

/**
 * Defines the structure search block.
 *
 * @param array $form
 *   An array representing a Drupal form.
 * @param array $form_state
 *   An array containing the Drupal form state.
 *
 * @return array
 *   A renderable array of a Drupal form.
 */
function islandora_chemistry_structure_search_form($form, &$form_state) {
  $chem_doodle_path = libraries_get_path('ChemDoodleWeb');
  $chem_path = drupal_get_path('module', 'islandora_chemistry');
  $element_id = drupal_html_id('islandora_chemistry_sketcher_canvas');
  $form['sketcher_canvas'] = array(
    '#type' => 'markup',
    '#attached' => array(
      'js' => array(
        array(
          'type' => 'setting',
          'data' => array(
            'islandora_chemistry_sketcher_canvas' => array(
              'element_id' => $element_id,
              'element_selector' => "#$element_id",
              'width' => 500,
              'height' => 500,
            ),
          ),
        ),
        "$chem_path/js/detectmobilebrowser.js",
        "$chem_doodle_path/install/ChemDoodleWeb.js",
        "$chem_doodle_path/install/uis/ChemDoodleWeb-uis.js",
        "$chem_path/js/sketchercanvas.js",
      ),
      'css' => array(
        "$chem_doodle_path/install/ChemDoodleWeb.css",
        "$chem_doodle_path/install/uis/jquery-ui-1.10.3.custom.css",
      ),
    ),
    '#markup' => "<canvas id='$element_id'></canvas>",
  );
  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Search'),
  );
  return $form;
}
