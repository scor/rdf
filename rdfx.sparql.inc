<?php
// $Id:  $

/**
 * @file
 * Setup for SPARQL queries.
 */

/**
 * Executes a SPARQL query on either a local SPARQL engine if available or
 * attempts to find a working public SPARQL endpoint on the web.
 *
 * @param $query
 *   The SPARQL query to execute.
 * @param $endpoint
 *   The endpoint to try the query against.
 * @options
 *   Options to use with the remote or local store.
 *      Remote:
 *        'store_read_key'
 * @param $datasets
 *   An array of datasets to query (optional). Implementation specific, i.e.
 *   ARC2 seems to require them to be pre-LOADed while public endpoints load
 *   FROM graphs automatically.
 * @return
 *   The SPARQL results.
 */
function rdfx_sparql_request($query, $endpoint = NULL, $options = array(), $datasets = array()) {
  // If an endpoint was provided, simply run this query against the endpoint.
  if ($endpoint != NULL) {
    $rows = _rdfx_sparql_request($query, $endpoint, $options);
    return $rows;
  }
  
  // Otherwise, load the datasets into either a local or remote endpoint.
  // @todo Check if the datasets are available before trying to query them.
  // Converts to array is string given.
  if (!is_array($datasets)) {
    $datasets = array($datasets);
  }

  // Attempts to execute the SPARQL query on a local ARC2 library.
  // MySQL is required.
  $db_spec = $GLOBALS['databases']['default']['default'];
  if ($db_spec['driver'] = 'mysql' && class_exists('ARC2')) {
    // Sets up a local repository which is required for running SPARQL queries
    // with ARC2.
    // @todo randomize table name to support concurrent querying.
    $store = rdfx_arc2_init_store('rdfx_sparql_tmp');

    // Load each dataset in the store.
    foreach ($datasets as $url) {
      // @todo LOAD INTO <...>
      $q = "LOAD <$url>";
      $store->query($q);
    }
    $rs = $store->query($query);
    if (!$store->getErrors()) {
      $rows = $rs['result']['rows'];
    }
  }
  // Try to execute the query against a series of public SPARQL endpoints.
  else {
    $endpoints = array(
      'http://www.sparql.org/sparql',
      'http://demo.openlinksw.com/sparql/',
    );

    foreach ($endpoints as $endpoint) {
      $rows = _rdfx_sparql_request($query, $endpoint, $options);
      if (!empty($rows)) {
        // If we received results, stop trying.
        break;
      }
    }
  }

  return $rows;
}

/**
 * Runs a query against an endpoint.
 *
 * @param $query
 *   The SPARQL query to execute.
 * @param $endpoint
 *   The endpoint to try the query against.
 * @options
 *   Options to use with the remote or local store, as passed to
 *   rdfx_sparql_request.
 */
function _rdfx_sparql_request($query, $endpoint, $options) {
  // Initialize connection with the endpoint.
  $store = rdfx_arc2_init_remote_store($endpoint, $options);

  // Execute the query.
  $rs = $store->query($query);
  if (!$store->getErrors()) {
    // Success!
    return $rs;
  }
  
  return NULL;
}

/**
 * Sets up an ARC2 RDF local repository.
 *
 * @param $name
 *   The name of the local repository.
 * @param $endpoint
 *   Set to TRUE if this store should also be setup as a SPARQL endpoint.
 * @return
 *   An ARC2 store object.
 */
function rdfx_arc2_init_store($name, $endpoint = FALSE) {
  // @todo fix this. Error reporting is off because ARC2 throws strict warnings.
  error_reporting(0);
  
  $db_spec = $GLOBALS['databases']['default']['default'];

  $config = array(
    /* db */
    'db_name' => $db_spec['database'],
    'db_user' => $db_spec['username'],
    'db_pwd' => isset($db_spec['password']) ? $db_spec['password'] : '',
    /* store */
    'store_name' => $name,

    /* endpoint */
    'endpoint_features' => array(
      'select', 'construct', 'ask', 'describe',
      //'load', 'insert', 'delete',
      'dump' /* dump is a special command for streaming SPOG export */
    ),
    'endpoint_timeout' => 60, /* not implemented in ARC2 preview */
    'endpoint_read_key' => '', /* optional */
    'endpoint_write_key' => '', /* optional */
    'endpoint_max_limit' => 500, /* optional */
  );

  // If this site is exposing a SPARQL endpoint, instantiate the endpoint with
  // the endpoint class which can be used for HTTP-based data access.
  if ($endpoint) {
    $store = ARC2::getStoreEndpoint($config);
  }
  // Otherwise, instantiate a locally accessible store.
  else {
    $store = ARC2::getStore($config);
  }
  if (!$store->isSetUp()) {
    $store->setUp();
  }

  return $store;
}

/**
 * Sets up an ARC2 RDF remote store.
 *
 * @param $endpoint
 *   The remote SPARQL endpoint.
 * @return
 *   An ARC2 store object.
 */
function rdfx_arc2_init_remote_store($endpoint, $options) {
  /* configuration */
  $config = array(
    /* remote endpoint */
    'remote_store_endpoint' => $endpoint,
    'store_read_key' => $options['store_read_key'] ? $options['store_read_key'] : '',
  );

  /* instantiation */
  $store = ARC2::getRemoteStore($config);

  return $store;
}