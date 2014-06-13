<?php

/**
 * @file
 * Handles the display / submission of the admin settings form for this module.
 */

/**
 * Defines the admin settings form.
 *
 * @param array $form
 *   The Drupal form definition.
 * @param array $form_state
 *   The Drupal form state.
 *
 * @return array
 *   The Drupal form definition.
 */
function islandora_chemistry_admin_settings_form(array $form, array &$form_state) {
  form_load_include($form_state, 'inc', 'islandora_chemistry', 'includes/admin.form');
  module_load_include('inc', 'islandora', 'includes/utilities');
  module_load_include('inc', 'islandora_chemistry', 'includes/utilities');
  $get_default_value = function($name, $default) use(&$form_state) {
    return isset($form_state['values'][$name]) ? $form_state['values'][$name] : variable_get($name, $default);
  };
  $python = $get_default_value('islandora_chemistry_python', '/usr/bin/python');
  $obabel = $get_default_value('islandora_chemistry_openbabel', '/usr/bin/obabel');
  $obprop = $get_default_value('islandora_chemistry_obprop', '/usr/bin/obprop');
  $cmmmsrv_host = $get_default_value('islandora_chemistry_cmmmsrv_host', 'localhost');
  $cmmmsrv_port = $get_default_value('islandora_chemistry_cmmmsrv_port', 55624);
  $inchi_field = $get_default_value(
    'islandora_chemistry_inchikey_solr_field',
    'cml_molecule_metadataList_mods_identifier_inchikey_ms'
  );
  $fragment_field = $get_default_value(
    'islandora_chemistry_fragment_solr_field',
    'cml_molecule_metadataList_mods_note_functional fragment_ms'
  );
  $form += array(
    'islandora_chemistry_openbabel' => array(
      '#prefix' => '<div id="obabel-wrapper">',
      '#suffix' => '</div>',
      '#type' => 'textfield',
      '#title' => t('OpenBabel'),
      '#description' => t('OpenBabel executable path<br/>!msg', array(
                        '!msg' => islandora_executable_available_message($obabel))),
      '#default_value' => $obabel,
      '#ajax' => array(
        'callback' => 'islandora_chemistry_admin_settings_form_ajax_callback',
        'wrapper' => 'obabel-wrapper',
        'effect' => 'fade',
        'event' => 'change',
      ),
    ),
    'islandora_chemistry_obprop' => array(
      '#prefix' => '<div id="obprop-wrapper">',
      '#suffix' => '</div>',
      '#type' => 'textfield',
      '#title' => t('OpenBabel Property Extractor'),
      '#description' => t('"obprop" executable path<br/>!msg', array(
                        '!msg' => islandora_executable_available_message($obprop))),
      '#default_value' => $obprop,
      '#ajax' => array(
        'callback' => 'islandora_chemistry_admin_settings_form_ajax_callback',
        'wrapper' => 'obprop-wrapper',
        'effect' => 'fade',
        'event' => 'change',
      ),
    ),
    'islandora_chemistry_python' => array(
      '#prefix' => '<div id="python-wrapper">',
      '#suffix' => '</div>',
      '#type' => 'textfield',
      '#title' => t('Python'),
      '#description' => t('Python executable path<br/>!msg', array(
                        '!msg' => islandora_executable_available_message($python))),
      '#default_value' => $python,
      '#ajax' => array(
        'callback' => 'islandora_chemistry_admin_settings_form_ajax_callback',
        'wrapper' => 'python-wrapper',
        'effect' => 'fade',
        'event' => 'change',
      ),
    ),
    'islandora_chemistry_cmmmsrv' => array(
      '#type' => 'fieldset',
      '#title' => t('Checkmol/Matchmol Server Information'),
      '#description' => filter_xss(@islandora_chemistry_admin_cmmmsrv_available($cmmmsrv_host, $cmmmsrv_port, 5) ?
        t('!imgThe server is available.', array(
          '!img' => theme_image(array('path' => 'misc/watchdog-ok.png', 'attributes' => array())),
        )) :
        t('!imgCould not connect to the given server.', array(
          '!img' => theme_image(array('path' => 'misc/watchdog-error.png', 'attributes' => array())),
        ))),
      '#prefix' => '<div id="cmmmsrv-wrapper">',
      '#suffix' => '</div>',
      'islandora_chemistry_cmmmsrv_host' => array(
        '#type' => 'textfield',
        '#title' => t('Checkmol/Matchmol Server Host'),
        '#description' => t('Hostname/IP address corresponding to where the cmmmsrv daemon is available.'),
        '#default_value' => $cmmmsrv_host,
        '#ajax' => array(
          'callback' => 'islandora_chemistry_admin_settings_form_cmmmsrv_ajax_callback',
          'wrapper' => 'cmmmsrv-wrapper',
          'effect' => 'fade',
          'event' => 'change',
        ),
      ),
      'islandora_chemistry_cmmmsrv_port' => array(
        '#type' => 'textfield',
        '#title' => t('Checkmol/Matchmol Server Port'),
        '#description' => t('TCP port on the given hostname/IP address corresponding to where the cmmmsrv daemon is available.'),
        '#default_value' => $cmmmsrv_port,
        '#ajax' => array(
          'callback' => 'islandora_chemistry_admin_settings_form_cmmmsrv_ajax_callback',
          'wrapper' => 'cmmmsrv-wrapper',
          'effect' => 'fade',
          'event' => 'change',
        ),
      ),
    ),
    'islandora_chemistry_search' => array(
      '#type' => 'fieldset',
      '#title' => t('Solr Search Fields'),
      '#collapsed' => TRUE,
      '#collapsible' => FALSE,
      'islandora_chemistry_inchikey_solr_field' => array(
        '#type' => 'textfield',
        '#title' => t('InChIKey Solr Field'),
        '#default_value' => $inchi_field,
      ),
      'islandora_chemistry_fragment_solr_field' => array(
        '#type' => 'textfield',
        '#title' => t('Fragment Solr Field'),
        '#default_value' => $fragment_field,
      ),
    ),
  );

  return system_settings_form($form);
}

/**
 * Ajax callback for the executable path textfields.
 *
 * @param array $form
 *   The Drupal form definition.
 * @param array $form_state
 *   The Drupal form state.
 *
 * @return array
 *   The element to render as part the ajax callback.
 */
function islandora_chemistry_admin_settings_form_ajax_callback(array $form, array $form_state) {
  return drupal_array_get_nested_value($form, $form_state['triggering_element']['#parents']);
}

/**
 * Ajax callback for the cmmmsrv textfields.
 *
 * @param array $form
 *   The Drupal form definition.
 * @param array $form_state
 *   The Drupal form state.
 *
 * @return array
 *   The element to render as part the ajax callback.
 */
function islandora_chemistry_admin_settings_form_cmmmsrv_ajax_callback(array $form, array $form_state) {
  return drupal_array_get_nested_value($form, array('islandora_chemistry_cmmmsrv'));
}

/**
 * Probe a hostname/port pair, to see if it correspond to a cmmmsrv daemon.
 *
 * @param string $host
 *   Address of the host to test.
 * @param int $port
 *   TCP port to test.
 *
 * @return bool
 *   TRUE if it appeared that we could connect, and it responded in a manner
 *   similar to that of a cmmmsrv daemon in a timely fashion (about five
 *   seconds); otherwise, FALSE.
 */
function islandora_chemistry_admin_cmmmsrv_available($host, $port) {
  module_load_include('inc', 'islandora_chemistry', 'includes/commands/checkmol');
  try {
    $cm = new \Islandora\Chemistry\Checkmol($host, $port, 5);
    return $cm->ready();
  }
  catch (\Islandora\Chemistry\IOException $e) {
    return FALSE;
  }
}
