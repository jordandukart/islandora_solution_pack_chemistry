<?php

/**
 * @file
 * Holds helper functions for the Islandora Chemistry Solution Pack.
 */

/**
 * Get a select drop down form element for selecting a chemistry MIME type.
 *
 * @return array
 *   A form select element for chemistry MIME types.
 */
function islandora_chemistry_mime_selector() {
  return array(
    '#title' => t('File Type'),
    '#description' => t('Select the file type for uploaded file(s).'),
    '#type' => 'select',
    '#options' => islandora_chemistry_get_chemistry_mimes(),
  );
}

/**
 * Get chemistry MIME types.
 *
 * @return array
 *   An array with MIME types as keys and human readable names as values.
 */
function islandora_chemistry_get_chemistry_mimes() {
  return array(
    'chemical/x-mdl-molfile' => 'MDL Molfile',
    'chemical/x-xyz' => 'XYZ format',
    'chemical/x-pdb' => 'PDB',
    'chemical/x-cdx' => 'ChemDraw CDX',
    'chemical/x-chem3d' => 'ChemDraw 3D',
    'chemical/x-chemdraw' => 'ChemDraw file',
    'chemical/x-cif' => 'Crystallographic Information File',
    'chemical/x-cml' => 'Chemical Markup Language',
    'chemical/x-gamess-input' => 'GAMESS Input',
    'chemical/x-gamess-output' => 'GAMESS Output',
    'chemical/x-gaussian-cube' => 'Gaussian Cube',
    'chemical/x-gaussian-input' => 'Gaussian 98/03 Cartesian Input',
    'chemical/x-gaussian-log' => 'Gaussian 98/03 Output',
    'chemical/x-jcamp-dx' => 'JCAMP Spectroscopic Data Exchange Format or OpenDX Grid',
    'chemical/x-mopac-input' => 'MOPAC Cartesian',
  );
}
