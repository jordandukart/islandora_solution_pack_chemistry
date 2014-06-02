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
    '#options' => array(
      'MDL Molfile' => 'chemical/x-mdl-molfile',
      'XYZ format' => 'chemical/x-xyz',
      'PDB' => 'chemical/x-pdb',
      'ChemDraw CDX' => 'chemical/x-cdx',
      'ChemDraw 3D' => 'chemical/x-chem3d',
      'ChemDraw file' => 'chemical/x-chemdraw',
      'Crystallographic Information File' => 'chemical/x-cif',
      'Chemical Markup Language' => 'chemical/x-cml',
      'GAMESS Input ' => 'chemical/x-gamess-input',
      'GAMESS Output' => 'chemical/x-gamess-input',
      'Gaussian Cube' => 'chemical/x-gaussian-cube',
      'Gaussian 98/03 Cartesian Input ' => 'chemical/x-gaussian-input',
      'Gaussian 98/03 Output' => 'chemical/x-gaussian',
      'JCAMP Spectroscopic Data Exchange Format' => 'chemical/x-jcamp-dx',
      'OpenDX Grid' => 'chemical/x-jcamp-dx',
      'MOPAC Cartesian' => 'chemical/x-mopac-input',
    ),
  );
}
