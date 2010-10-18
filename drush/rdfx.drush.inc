<?php
// $Id:  $

/**
 * @file
 *   drush integration for rdfx.
 */

/**
 * Implementation of hook_drush_command().
 *
 * In this hook, you specify which commands your
 * drush module makes available, what it does and
 * description.
 *
 * Notice how this structure closely resembles how
 * you define menu hooks.
 *
 * @See drush_parse_command() for a list of recognized keys.
 *
 * @return
 *   An associative array describing your command(s).
 */
function rdfx_drush_command() {
  $items = array();

  $items['rdf-download'] = array(
    'callback' => 'rdfx_drush_arc2_download',
    'description' => dt('Downloads the required ARC2 library from http://arc.semsol.org/download'),
    'aliases' => array('rdfdl'),
    'arguments' => array(
      'path' => dt('Optional. A path to the rdfx module. If omitted Drush will use the default location.'),
    ),
  );
  return $items;
}

/**
 * Implementation of hook_drush_help().
 *
 * This function is called whenever a drush user calls
 * 'drush help <name-of-your-command>'
 *
 * @param
 *   A string with the help section (prepend with 'drush:')
 *
 * @return
 *   A string with the help text for your command.
 */
function rdfx_drush_help($section) {
  switch ($section) {
    case 'drush:rdf-download':
      return dt("Downloads the required ARC2 library from http://arc.semsol.org/download. Include the optional path.");
  }
}

/**
 * Example drush command callback.
 *
 * This is where the action takes place.
 *
 * In this function, all of Drupals API is (usually) available, including
 * any functions you have added in your own modules/themes.
 *
 * To print something to the terminal window, use drush_print().
 *
 */
function rdfx_drush_arc2_download() {
  $args = func_get_args();
  if ($args[0]) {
    $path = $args[0];
  }
  elseif(module_exists('libraries')) {
    $path = 'sites/all/libraries';

    // Create the path if it does not exist.
    if (!is_dir($path)) {
      drush_op('mkdir', $path);
      drush_log(dt('Directory @path was created', array('@path' => $path)), 'notice');
    }
  }
  else {
    $path = drupal_get_path('module', 'rdfx') . '/vendor';
    // Create vendor directory if it does not exist yet.
    if (!drush_op('chdir', $path)) {
      drush_op('mkdir', $path);
    }
  }
  drush_op('chdir', $path);
  // Download and extract library.
  if (drush_shell_exec('wget http://code.semsol.org/source/arc.tar.gz') &&
      drush_shell_exec('tar zxvf arc.tar.gz') &&
      drush_shell_exec('rm arc.tar.gz')) {
    drush_log(dt('The latest ARC2 library has been downloaded to @path', array('@path' => $path)), 'success');
  }
  else {
    drush_log(dt('Drush was unable to download the ARC2 library to @path', array('@path' => $path)), 'error');
  }
}
