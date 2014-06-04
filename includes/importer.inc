<?php
/**
 * @file
 * Defines the IslandoraSerialObjectZipPDFBatchImporter.
 */

/**
 * Defines the ZIP PDF importer for Islandora Serial Object.
 */
class IslandoraChemistryZipBatchImporter extends ZipBatchImporter {
  protected $itemClass = 'IslandoraChemistryZipBatchImportObject';

  /**
   * Get the required info to pass into the file parameter of the constructor.
   *
   * @see IslandoraBatchImporter::getBatchInfo()
   */
  public static function getBatchInfo(array &$params) {
    module_load_include('inc', 'islandora_serial_object', 'includes/utilities');
    $params['object_info'] = static::getIngestInfo($params['file']);
    return $params;
  }

  /**
   * Get ingest info.
   *
   * @see IslandoraBatchImporter::getIngestInfo()
   */
  protected static function getIngestInfo($file) {
    module_load_include('inc', 'islandora_chemistry', 'includes/utilities');
    $zip = new ZipArchive();
    if (($error = $zip->open(drupal_realpath($file->uri))) !== TRUE) {
      drupal_set_message(t('Error opening the provided Zip file. Code: %code', array(
        '%code' => $error,
      )));
      return;
    }

    $count = $zip->numFiles;

    $info = array();
    for ($i = 0; $i < $count; $i++) {
      $name = $zip->getNameIndex($i);
      $path_info = pathinfo($name);

      $object_key = $path_info['dirname'] . '-_-' . $path_info['filename'];
      if (strpos($path_info['dirname'], '__MACOSX') === FALSE
        && isset($path_info['extension'])
        && $path_info['extension'] != 'DS_Store') {
        $datastream_key = strtolower($path_info['extension']);
        $info[$object_key][$datastream_key] = $name;
      }
    }
    $zip->close();
    return $info;
  }
}

/**
 * ZIP import object.
 *
 * Actually does the heavy-lifting during the import.
 */
class IslandoraChemistryZipBatchImportObject extends ZipBatchImportObject {
  /**
   * Constructor.
   */
  protected function __construct($source) {
    parent::__construct($source);
    $this->mimeType = $source['mimetype'];
  }

  /**
   * Get an item from the source.
   *
   * @see IslandoraImportObject::getOne()
   */
  public static function getOne(&$info) {
    $record = array(
      'pid_namespace' => $info['pid_namespace'],
      'file' => $info['file'],
      'content_model' => $info['content_model'],
      'mimetype' => $info['file_mime'],
    );
    $key = key($info['object_info']);
    $record['object_info'] = $info['processed_objects'][$key] = $info['object_info'][$key];
    unset($info['object_info'][$key]);
    return (empty($record) ? FALSE : new static($record));
  }

  /**
   * Acquire base datastreams structure.
   *
   * @see IslandoraImportObject::getDatastreams()
   */
  protected function getDatastreams(&$errors = NULL, &$files = NULL) {
    $to_return = array();
    foreach ($this->source['object_info'] as $datastream => $name) {
      $zip = new ZipArchive();
      $zip->open(drupal_realpath($this->source['file']->uri));
      if ($zip->extractTo(drupal_realpath('public://'), $name)) {
        $file = new stdClass();
        $file->uri = "public://$name";
        $files[] = $file;

        $to_return += array(
          'OBJ' => array(
            'label' => basename($name),
            'dsid' => 'OBJ',
            'mimetype' => $this->mimeType,
            'datastream_file' => file_create_url($file->uri),
            'filename' => $name,
            'control_group' => 'M',
            'file' => $file,
          ),
        );
      }
      $zip->close();
    }
    return $to_return;
  }
}
