<?php

/**
 * @file
 * Defines a class for interacting with OpenBabel.
 */

namespace Islandora\Chemistry\OpenBabel;
use ArrayAccess,
    InvalidArgumentException,
    OutOfBoundsException;

/**
 * Class Options
 *
 * Models options for the OpenBabel CLI.
 *
 * @package Islandora\OpenBabel
 */
class Options implements ArrayAccess {

  /**
   * Defines the possible options and what values are acceptable.
   *
   * Each array key represents a valid option for the, its value is
   * must be an associative array with the following keys:
   *  - type: Expected to be either 'boolean', 'string', 'integer', 'select',
   *    the type defines default validation and serialization functions for the
   *    option. Required.
   *  - multiple: If present the option will be stored as an array and the user
   *    will be add multiple values for the option. Optional.
   *  - options: An array of values that the option is limited to, this field is
   *    required when the type is 'select'. For other types it has no affect.
   *  - validate: A function that takes an option name and value and returns a
   *    boolean value indicating if that option value pair is valid.
   *  - serialize: A function that takes an option name and value and returns
   *    the string representation of that option.
   *  - length: Whether this is a 'short' (one hyphen) or 'long' (two hyphens)
   *    option.
   *
   * Some assumptions are made, about how each option will be serialized and
   * validated based on it's type. Options can give explicit validation /
   * serialization functions though.
   *
   * @var array
   */
  protected static $optionDefinitions = array(
    ':' => array(
      'type' => 'string',
      'length' => 'short',
    ),
    'a' => array(
      'type' => 'string',
      'length' => 'short',
    ),
    'addtotitle' => array(
      'type' => 'boolean',
      'length' => 'long',
    ),
    'addformula' => array(
      'type' => 'boolean',
      'length' => 'long',
    ),
    'b' => array(
      'type' => 'boolean',
      'length' => 'short',
    ),
    'c' => array(
      'type' => 'boolean',
      'length' => 'short',
    ),
    'C' => array(
      'type' => 'boolean',
      'length' => 'short',
    ),
    'd' => array(
      'type' => 'boolean',
      'length' => 'short',
    ),
    'errorlevel' => array(
      'type' => 'select',
      'length' => 'long',
      'options' => array(1, 2, 3, 4, 5),
    ),
    'f' => array(
      'type' => 'integer',
      'length' => 'long',
    ),
    'h' => array(
      'type' => 'boolean',
      'length' => 'short',
    ),
    'i' => array(
      'type' => 'select',
      'length' => 'short',
      'options' => array(
        'acr',
        'adfout',
        'alc',
        'arc',
        'bgf',
        'box',
        'bs',
        'c3d1',
        'c3d2',
        'caccrt',
        'can',
        'car',
        'castep',
        'ccc',
        'cdx',
        'cdxml',
        'cif',
        'ck',
        'cml',
        'cmlr',
        'CONFIG',
        'CONTCAR',
        'crk2d',
        'crk3d',
        'ct',
        'cub',
        'cube',
        'dat',
        'dmol',
        'dx',
        'ent',
        'fa',
        'fasta',
        'fch',
        'fchk',
        'fck',
        'feat',
        'fhiaims',
        'fract',
        'fs',
        'fsa',
        'g03',
        'g09',
        'g92',
        'g94',
        'g98',
        'gal',
        'gam',
        'gamess',
        'gamin',
        'gamout',
        'got',
        'gpr',
        'gukin',
        'gukout',
        'gzmat',
        'hin',
        'HISTORY',
        'inchi',
        'inp',
        'ins',
        'jout',
        'log',
        'mcdl',
        'mcif',
        'mdl',
        'ml2',
        'mmcif',
        'mmd',
        'mmod',
        'mol',
        'mol2',
        'mold',
        'molden',
        'molf',
        'moo',
        'mop',
        'mopcrt',
        'mopin',
        'mopout',
        'mpc',
        'mpo',
        'mpqc',
        'mrv',
        'msi',
        'nwo',
        'out',
        'outmol',
        'output',
        'pc',
        'pcm',
        'pdb',
        'pdbqt',
        'png',
        'POSCAR',
        'pqr',
        'pqs',
        'prep',
        'pwscf',
        'qcout',
        'res',
        'rsmi',
        'rxn',
        'sd',
        'sdf',
        'smi',
        'smiles',
        'sy2',
        't41',
        'tdd',
        'text',
        'therm',
        'tmol',
        'txt',
        'unixyz',
        'vmol',
        'xml',
        'xtc',
        'xyz',
        'yob',
      ),
    ),
    'j' => array(
      'type' => 'boolean',
      'length' => 'short',
    ),
    'join' => array(
      'type' => 'boolean',
      'length' => 'long',
    ),
    'k' => array(
      'type' => 'boolean',
      'length' => 'short',
    ),
    'l' => array(
      'type' => 'integer',
      'length' => 'short',
    ),
    'o' => array(
      'type' => 'select',
      'length' => 'short',
      'options' => array(
        'adf',
        'alc',
        'bgf',
        'box',
        'bs',
        'c3d1',
        'c3d2',
        'cac',
        'caccrt',
        'cache',
        'cacint',
        'can',
        'cdxml',
        'cht',
        'cif',
        'ck',
        'cml',
        'cmlr',
        'com',
        'CONFIG',
        'copy',
        'crk2d',
        'crk3d',
        'csr',
        'cssr',
        'ct',
        'cub',
        'cube',
        'dmol',
        'dx',
        'ent',
        'fa',
        'fasta',
        'feat',
        'fh',
        'fhiaims',
        'fix',
        'fpt',
        'fract',
        'fs',
        'fsa',
        'gamin',
        'gau',
        'gjc',
        'gjf',
        'gpr',
        'gr96',
        'gukin',
        'gukout',
        'gzmat',
        'hin',
        'inchi',
        'inp',
        'jin',
        'k',
        'mcdl',
        'mcif',
        'mdl',
        'ml2',
        'mmcif',
        'mmd',
        'mmod',
        'mna',
        'mol',
        'mol2',
        'mold',
        'molden',
        'molf',
        'molreport',
        'mop',
        'mopcrt',
        'mopin',
        'mp',
        'mpc',
        'mpd',
        'mpqcin',
        'mrv',
        'msms',
        'nul',
        'nw',
        'outmol',
        'pcm',
        'pdb',
        'pdbqt',
        'png',
        'pov',
        'pqr',
        'pqs',
        'qcin',
        'report',
        'rsmi',
        'rxn',
        'sd',
        'sdf',
        'smi',
        'smiles',
        'svg',
        'sy2',
        'tdd',
        'therm',
        'tmol',
        'txt',
        'txyz',
        'unixyz',
        'vmol',
        'xed',
        'xyz',
        'yob',
        'zin',
      ),
    ),
    'O' => array(
      'type' => 'string',
      'length' => 'short',
    ),
    'p' => array(
      'type' => 'boolean',
      'length' => 'short',
    ),
    'property' => array(
      'type' => 'boolean',
      'length' => 'long',
    ),
    's' => array(
      'type' => 'string',
      'length' => 'short',
    ),
    'separate' => array(
      'type' => 'boolean',
      'length' => 'long',
    ),
    't' => array(
      'type' => 'boolean',
      'length' => 'short',
    ),
    'title' => array(
      'type' => 'string',
      'length' => 'long',
    ),
    'x' => array(
      'type' => 'string',
      'length' => 'short',
    ),
    'v' => array(
      'type' => 'string',
      'length' => 'short',
    ),
    'V' => array(
      'type' => 'boolean',
      'length' => 'short',
    ),
    'z' => array(
      'type' => 'boolean',
      'length' => 'short',
    ),
  );

  /**
   * Options for the OpenBabel CLI.
   *
   * The options in this array must conform to the restrictions given in
   * $valid_options.
   *
   * @var array
   */
  protected $options = array();

  /**
t
   * Defines default option definitions based on the options type.
   *
   * The option definitions can optionally overwrite these defaults.
   *
   * @return array
   *   The default option definitions for all accepted types.
   */
  protected static function getDefaultTypeOptionDefinitions() {
    // @codingStandardsIgnoreStart
    static $default_option_type_definitions = NULL;
    if ($default_option_type_definitions === NULL) {
      // Call the given function with only the $value parameter. Used in the
      // simple validation callbacks.
      $call_without_option_function = function ($function) {
        return function ($option, $value) use ($function) {
          return $function($value);
        };
      };
      $default_option_type_definitions = array(
        'boolean' => array(
          'validate' => $call_without_option_function('is_bool'),
          'serialize' => array('\Islandora\Chemistry\OpenBabel\Options', 'defaultSerializeBoolean'),
        ),
        'string' => array(
          'validate' => $call_without_option_function('is_string'),
          'serialize' => array('\Islandora\Chemistry\OpenBabel\Options', 'defaultSerializeSingleValue'),
        ),
        'integer' => array(
          'validate' => $call_without_option_function('is_int'),
          'serialize' => array('\Islandora\Chemistry\OpenBabel\Options', 'defaultSerializeSingleValue'),
        ),
        'select' => array(
          'validate' => array('\Islandora\Chemistry\OpenBabel\Options', 'defaultValidateSelect'),
          'serialize' => array('\Islandora\Chemistry\OpenBabel\Options', 'defaultSerializeSingleValue'),
        ),
      );
    }
    return $default_option_type_definitions;
    // @codingStandardsIgnoreEnd
  }

  public static function defaultSerializeSingleValue($option, $value) {
    $def = static::$optionDefinitions[$option] + array(
      'length' => 'short',
    );
    return $def['length'] == 'short' ?
      "-{$option}{$value}" :
      "--{$option} \"{$value}\"";
  }
  public static function defaultSerializeBoolean($option, $value) {
    $def = static::$optionDefinitions[$option] + array(
      'length' => 'short',
    );
    return $value ?
      ($def['length'] == 'short' ?
        "-{$option}" :
        "--{$option}") :
      '';
  }
  public static function defaultValidateSelect($option, $value) {
    return in_array($value, static::$optionDefinitions[$option]['options']);
  }

  /**
   * Gets only the option defaults for the given type.
   *
   * @param string $type
   *   The type of option to get the defaults for.
   *
   * @return array
   *   The default option definition for the given type.
   */
  protected static function getDefaultTypeOptionDefinition($type) {
    $defaults = static::getDefaultTypeOptionDefinitions();
    return $defaults[$type];
  }

  /**
   * Checks to see if the given option exists for OpenBabel.
   *
   * @param string $option
   *   The option to check.
   *
   * @return bool
   *   TRUE if the given option exists, FALSE otherwise.
   */
  public static function exists($option) {
    return !empty(static::$optionDefinitions[$option]);
  }

  /**
   * Checks to see if the given option / value is valid.
   *
   * @param string $option
   *   The option to check.
   * @param mixed $value
   *   The option value to check.
   *
   * @return bool
   *   TRUE if the given option / value is valid, FALSE otherwise.
   */
  public static function valid($option, $value) {
    if (static::exists($option)) {
      $definition = static::$optionDefinitions[$option];
      $definition += static::getDefaultTypeOptionDefinition($definition['type']);
      return call_user_func($definition['validate'], $option, $value);
    }
    return FALSE;
  }

  /**
   * Checks to see if the given option allows multiple values.
   *
   * @param string $option
   *   The option to check.
   *
   * @return bool
   *   TRUE if the given option allows multiple values, FALSE otherwise.
   */
  public static function multiple($option) {
    return isset(static::$optionDefinitions[$option]['multiple']) && static::$optionDefinitions[$option]['multiple'];
  }

  /**
   * Serialize the given option and it's value to a string to pass to the CLI.
   *
   * @param string $option
   *   The option to serialize.
   * @param mixed $value
   *   The option value to serialize.
   *
   * @return string
   *   A string representing the given option and value.
   */
  public static function serialize($option, $value) {
    if (static::exists($option)) {
      $definition = static::$optionDefinitions[$option];
      $definition += static::getDefaultTypeOptionDefinition($definition['type']);
      // Serialize the given value using the serialize function in the option
      // definition.
      $serialize = function($value) use($option, $definition) {
        return call_user_func($definition['serialize'], $option, $value);
      };
      $multiple = static::multiple($option) && is_array($value);
      return ($multiple) ? implode(' ', array_map($serialize, $value)) : $serialize($value);
    }
    return FALSE;
  }

  /**
   * Create an new Islandora\OpenBabel\Options object.
   *
   * @throws InvalidArgumentException
   *
   * @param array $options
   *   Options to use. For what options are available see the manual pages.
   *   When passing options here you can omit the '--'. Options which can be
   *   specified multiple times are expected to be arrays.
   */
  public function __construct(array $options = array()) {
    foreach ($options as $option => $value) {
      // offsetSet() will throw an exception if the option or value is not
      // valid.
      $this[$option] = $value;
    }
  }

  /**
   * Magic offsetGet.
   *
   * This is implemented for multivalued options as offsetGet does not return by
   * reference, which is needed for multidimensional arrays.
   *
   * @see ArrayAccess::offsetGet
   */
  public function &__get($option) {
    if (isset($this->options[$option])) {
      return $this->options[$option];
    }
    else {
      throw new OutOfBoundsException($option);
    }
  }

  /**
   * Magic offsetSet.
   *
   * This is implemented for multivalued options as offsetGet does not return by
   * reference, which is need for multidimensional arrays.
   *
   * @see ArrayAccess::offsetGet
   */
  public function __set($option, $value) {
    $this->offsetSet($option, $value);
  }

  /**
   * Checks to see if the offset exists.
   *
   * @see ArrayAccess::offsetExists
   */
  public function offsetExists($option) {
    return isset($this->options[$option]);
  }

  /**
   * Custom offsetGet.
   *
   * We don't allow for indirect access to elements as our min version of PHP is
   * 5.3.3, and indirect access was not added until 5.3.4. Please use the __get
   * to do indirect access on options.
   * @see http://php.net/manual/en/arrayaccess.offsetget.php
   *
   * @see ArrayAccess::offsetGet
   */
  public function offsetGet($option) {
    if (isset($this->options[$option])) {
      return $this->options[$option];
    }
    else {
      throw new OutOfBoundsException($option);
    }
  }

  /**
   * Custom offsetSet.
   *
   * @see ArrayAccess::offsetSet
   */
  public function offsetSet($option, $value) {
    // Check if the $option is valid.
    if (!static::exists($option)) {
      throw new InvalidArgumentException('Unknown command line option ' . $option);
    }
    // Validate the given single value.
    $validate = function ($value) use ($option) {
      if (!Options::valid($option, $value)) {
        throw new InvalidArgumentException("Invalid value '{$value}' given for command line option '{$option}'");
      }
    };
    // Multiple valued options must always be stored as arrays.
    $value = (static::multiple($option) && !is_array($value)) ? array($value) : $value;
    // Validate the given value / values.
    is_array($value) ? array_walk($value, $validate) : $validate($value);
    // No exceptions? Assign.
    $this->options[$option] = $value;
  }

  /**
   * Custom offsetUnset.
   *
   * @see ArrayAccess::offsetUnset
   */
  public function offsetUnset($option) {
    unset($this->options[$option]);
  }

  /**
   * Enables the given option.
   *
   * @throws InvalidArgumentException
   *   If the given option can not be enabled.
   *
   * @param string $option
   *   Only use options which can be toggled on/off like "print-query".
   */
  public function enable($option) {
    if (!static::valid($option, TRUE)) {
      throw new InvalidArgumentException("{$option} can not be enabled");
    }
    $this->options[$option] = TRUE;
  }

  /**
   * Disables the given option.
   *
   * @throws InvalidArgumentException
   *   If the given option can not be disabled.
   *
   * @param string $option
   *   Only use options which can be toggled on/off like "print-query".
   */
  public function disable($option) {
    if (!static::valid($option, FALSE)) {
      throw new InvalidArgumentException("{$option} can not be disabled");
    }
    unset($this->options[$option]);
  }

  /**
   * Sets the given option to the given value.
   *
   * @throws InvalidArgumentException
   *   If the given option can not be enabled.
   *
   * @param string $option
   *   The option to set.
   *
   * @param mixed $value
   *   The value to set the option.
   */
  public function set($option, $value) {
    if (!static::valid($option, $value)) {
      throw new InvalidArgumentException("{$option} can not be set to {$value}");
    }
    $this->options[$option] = $value;
  }

  /**
   * Generates a string representation.
   *
   * @return string
   *   The representation of this object.
   */
  public function __toString() {
    $output = '';
    foreach ($this->options as $option => $value) {
      $output .= static::serialize($option, $value) . ' ';
    }
    return trim($output);
  }

}

/**
 * Calls the OpenBabel CLI with the stored options.
 *
 * @param string $filename
 *   A filename to be transformed.
 * @param Options $options
 *   The options to pass to the OpenBabel CLI.
 *
 * @return string
 *   The output of the command executed.
 */
function execute($file, Options $options, $executable_path = NULL) {
  if ($executable_path === NULL ) {
    $executable_path = variable_get('islandora_chemistry_openbabel', '/usr/bin/obabel');
  }

  if (is_executable($executable_path)) {
    $ret = -1;
    $output = array();
    $command = "$executable_path $file $options";
    $desc_spec = array(
      0 => array('pipe', 'r'),
      1 => array('pipe', 'w'),
      2 => array('pipe', 'w')
    );
    $proc = proc_open($command, $desc_spec, $pipes);
    fclose($pipes[0]);
    fclose($pipes[2]);
    $output = stream_get_contents($pipes[1]);
    fclose($pipes[1]);
    $ret = proc_close($proc);
    if ($ret == 0) {
      return trim($output);
    }
  }
  return FALSE;
}
