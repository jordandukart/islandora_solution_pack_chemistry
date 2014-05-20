<?php

class IslandoraOBabel {
  protected $islandoraObject;

  public function __construct($pid) {
    module_load_include('inc', 'islandora', 'includes/utilities');
    $this->islandoraObject = islandora_object_load($pid);
  }

  /**
   * Function to create a standard MOL file from whatever format is ingested.
   *
   * @param type $dsid
   * @param type $file
   * @param type $file_ext
   * @return string
   */
  public static function chem2mol($dsid, $file, $file_ext) {
    module_load_include('inc', 'islandora_chemistry', 'FileConversion');
    $file_suffix = '-' . $dsid . '.' . $file_ext;
    $returnValue = TRUE;
    $filename = $file . $file_suffix;
    $filename = str_replace(" ", "-", $filename);
    //clean_XYZ($file);
//  Call openbabel and convert the file to MOL. (No longer needed - Create the 3D structure so that it looks good in the 3D viewer)
    if (!file_exists($file . "svg")) {
      system("obabel \"$file\" --title -omol -xw -O\"$filename\"", $returnValue);
    }
    else {
      $returnValue = '0';
    }

    $returnValue = '0';

    if ($returnValue == '0') {
      $_SESSION['fedora_ingest_files']["$dsid"] = $filename;
      return TRUE;
    }
    else {
      return $returnValue;
    }
  }
}