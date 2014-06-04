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
  $python = $get_default_value('islandora_chemistry_python', 'python');
  $obabel = $get_default_value('islandora_chemistry_openbabel', 'obabel');
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
