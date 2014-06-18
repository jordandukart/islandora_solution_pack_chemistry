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
  $options['#options']['other'] = t('Other');
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
    if (in_array($chem_file_mime, array('application/x-zip', 'application/zip'))) {
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

  if ($form_state['values']['type_select_check'] && $form_state['values']['options'] == 'other') {
    $content_model = 'islandora:binaryObjectCModel';
    if (in_array($file->filemime, array('application/x-zip', 'application/zip'))) {
      $mimetype = 'application/octet-stream';
    }
    else {
      $mimetype = $file->filemime;
    }
  }
  else {
    $content_model = 'islandora:sp_chem_CM';
    $mimetype = $form_state['values']['type_select_check'] ? $form_state['values']['options'] : $file->filemime;
  }

  if (in_array($file->filemime, array('application/x-zip', 'application/zip'))) {
    $params = array(
      'file' => file_load($form_state['values']['upload_file']),
      'pid_namespace' => $default_namespace,
      'content_model' => array($content_model),
      'processed_objects' => array(),
      'file_mime' => $mimetype,
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
    $object->models = array($content_model);
    $ds = $object->constructDatastream('OBJ', 'M');
    $object->ingestDatastream($ds);
    $ds->label = $file->filename;
    $ds->mimetype = $mimetype;
    $ds->setContentFromFile($file->uri, FALSE);
    $tuque->repository->ingestObject($object);
    drupal_set_message(t(
      'Object ingested at <a href="@url">@id</a>',
      array('@url' => url("islandora/object/{$object->id}"), '@id' => $object->id)
    ));
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
  // Grab mol from session if available.
  $inchikey_solr_field = islandora_solr_lesser_escape(
    variable_get(
      'islandora_chemistry_inchikey_solr_field',
      'cml_molecule_metadataList_mods_identifier_inchikey_ms'
    )
  );
  $matches = array();
  $match = preg_match("/$inchikey_solr_field\:\"(.*)\"/", current_path(), $matches);
  if ($match) {
    $inchikey = $matches[1];
  }
  if (isset($inchikey)) {
    if (isset($_SESSION['islandora_chemistry_inchikeys'][$inchikey])) {
      $mol = $_SESSION['islandora_chemistry_inchikeys'][$inchikey];
    }
  }
  // XXX There is a small chance of leaking memory from this cache if an
  // exception occurs between the submit handler and building the form. More
  // utility can be gained to including the mol files in the db.
  if (isset($_SESSION['islandora_chemistry_inchikeys']) &&
    count($_SESSION['islandora_chemistry_inchikeys']) >= 5) {
    array_shift($_SESSION['islandora_chemistry_inchikeys']);
  }
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
              'hidden_field_selector' => "#islandora-chemistry-structure-search-form input[name=mol_value]",
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
  $form['mol_value'] = array(
    '#type' => 'hidden',
    '#default_value' => isset($mol) ? $mol : NULL,
  );
  $form['search_type'] = array(
    '#type' => 'radios',
    '#title' => t('Search type'),
    '#default_value' => 'inchi',
    '#options' => array(
      'inchi' => t('InChIKey'),
      'substructure' => t('Substructure'),
      'fragment' => t('Fragment'),
    ),
  );
  $form['inchi_description'] = array(
    '#type' => 'item',
    '#description' => t("Matches the drawn molecule's InChIKey to existing molecules."),
    '#states' => array(
      'visible' => array(
        ':input[name="search_type"]' => array('value' => 'inchi'),
      ),
    ),
  );
  $form['substructure_description'] = array(
    '#type' => 'item',
    '#description' => t('Matches all substructures contained in the drawn molecule.'),
    '#states' => array(
      'visible' => array(
        ':input[name="search_type"]' => array('value' => 'substructure'),
      ),
    ),
  );
  $form['fragment_description'] = array(
    '#type' => 'item',
    '#description' => t('Matches all fragments contained in the drawn molecule.'),
    '#states' => array(
      'visible' => array(
        ':input[name="search_type"]' => array('value' => 'fragment'),
      ),
    ),
  );
  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Search'),
    '#attributes' => array(
      'onclick' => 'Drupal.settings.islandora_chemistry_sketcher_canvas.grab_structure()',
    ),
  );
  return $form;
}

/**
 * Submit handler for the structure search block.
 *
 * @param array $form
 *   An array representing a Drupal form.
 * @param array $form_state
 *   An array containing the Drupal form state.
 */
function islandora_chemistry_structure_search_form_submit($form, &$form_state) {
  module_load_include('inc', 'islandora_chemistry', 'includes/commands/openbabel');
  module_load_include('inc', 'islandora_chemistry', 'includes/commands/checkmol');
  module_load_include('inc', 'islandora_chemistry', 'includes/utilities');
  module_load_include('inc', 'islandora_solr', 'includes/utilities');

  $mol = $form_state['values']['mol_value'];
  $inchi_key = islandora_chemistry_get_inchikey_from_mol($mol);
  $_SESSION['islandora_chemistry_inchikeys'][$inchi_key] = $mol;

  $search_type = $form_state['values']['search_type'];
  $inchi_key_solr_field = islandora_solr_lesser_escape(
    variable_get(
      'islandora_chemistry_inchikey_solr_field',
      'cml_molecule_metadataList_mods_identifier_inchikey_ms'
    )
  );
  if ($search_type == 'inchi') {
    $query = format_string('!inchi_field:"!inchi_key"', array(
      '!inchi_field' => $inchi_key_solr_field,
      '!inchi_key' => $inchi_key,
    ));
  }
  elseif ($search_type == 'fragment') {
    $options = new \Islandora\Chemistry\OpenBabel\Options(array(
      'i' => 'mol',
      'o' => 'mol',
    ));
    $options->setStdIn($mol);
    $cleaned_mol = \Islandora\Chemistry\OpenBabel\execute(
      FALSE,
      $options
    );
    $cm = new \Islandora\Chemistry\Checkmol(
      variable_get('islandora_chemistry_cmmmsrv_host', 'localhost'),
      variable_get('islandora_chemistry_cmmmsrv_port', 55624)
    );
    $fragment_solr_field = islandora_solr_lesser_escape(
      variable_get(
        'islandora_chemistry_fragment_solr_field',
        'cml_molecule_metadataList_mods_note_functional fragment_ms'
      )
    );
    $fragments = $cm->get8DigitCodes($cleaned_mol);
    $fragments = array_map('islandora_chemistry_prepare_record_for_solr', $fragments);
    $query = format_string('!fragment_field:(!fragments)', array(
      '!fragment_field' => $fragment_solr_field,
      '!fragments' => empty($fragments) ? '""' : implode(' OR ', $fragments),
    ));
  }
  else {
    module_load_include('inc', 'islandora_chemistry', 'includes/fingerprint');
    $options = new \Islandora\Chemistry\OpenBabel\Options(array(
      'i' => 'mol',
      'o' => 'mol',
    ));
    $options->setStdIn($mol);
    $cleaned_mol = \Islandora\Chemistry\OpenBabel\execute(
      FALSE,
      $options
    );

    // This retrieves all parts of the fingerprint from the table, or inserts
    // the new structure to avoid running identical calls in the future.
    $fingerprint_parts = islandora_chemistry_fingerprint($inchi_key, $cleaned_mol);
    $db_and = db_and();
    $count = 0;

    // Need to construct the condition like this as there's is no nice way of
    // handling bitwise conditions in Drupal's DB layer.
    foreach ($fingerprint_parts as $key => $val) {
      $db_and->where("$key & :val_$count=:val_$count", array(
        ":val_$count" => $val,
      ));
      $count++;
    }
    $inchi_keys = db_select('islandora_chemistry_fingerprints', 'i')
      ->fields('i', array('inchikey'))
      ->condition($db_and)
      ->execute()
      ->fetchCol();
    $inchi_keys = array_map('islandora_chemistry_prepare_record_for_solr', $inchi_keys);
    $query = format_string('!inchi_field:(!inchi_keys)', array(
        '!inchi_field' => $inchi_key_solr_field,
        '!inchi_keys' => implode(' OR ', $inchi_keys),
      ));
  }
  $form_state['redirect'] = "islandora/search/$query";
}

/**
 * Helper function to wrap a value in quotes for Solr.
 *
 * @param string $value
 *   The string to be quoted.
 *
 * @return string
 *   The quoted string.
 */
function islandora_chemistry_prepare_record_for_solr($value) {
  return format_string('"!value"', array('!value' => $value));
}
