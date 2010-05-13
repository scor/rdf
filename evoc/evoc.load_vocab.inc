<?php

/**
 * Loads an RDF/XML file from an HTTP URI, parses it, and builds an RDF schema
 * representation (associative array) from it.
 * Handles content negotiation and various errors.
 *
 * @param string $uri
 * @return array
 * @throws Exception On network or parse error
 */
function evoc_get_from_web($uri, $prefix) {
  $schema_url = $uri;
  $i = strpos($schema_url, '#');
  if ($i !== false) {
    $schema_url = substr($schema_url, 0, $i);
  }
  list($triples, $namespaces) = _evoc_parse_rdfxml($uri);
  return _evoc_extract_schema($triples, $namespaces, $prefix, $uri);
}

/**
 * Loads a local RDF/XML file, parses it, and builds
 * an RDF schema representation (associative array) from it.
 * Handles various errors.
 *
 * @param string $uri
 * @return array
 * @throws Exception On file or parse error
 */
function evoc_get_from_file($filename, $uri, $prefix) {
  $content = _evoc_fetch_from_file($filename);
  list($triples, $namespaces) = _evoc_parse_rdfxml($uri, $content);
  return _evoc_extract_schema($triples, $namespaces, $prefix, $uri);
}

/**
 * Loads the content of a local file.
 *
 * @param string $filename
 * @return string
 * @throws Exception
 */
function _evoc_fetch_from_file($filename) {
  if (!is_file($filename)) {
    throw new Exception("File not found: '$filename'");
  }
  return file_get_contents($filename);
}

function &_evoc_parse_rdfxml($base_uri) {
  include_once(drupal_get_path('module', 'rdfx') . '/vendor/arc/ARC2.php');
  $parser = ARC2::getRDFXMLParser();
  $parser->parse($base_uri);
  $namespaces = array();
  foreach ($parser->nsp as $uri => $prefix) {
    $namespaces[$prefix] = $uri;
  }
  return array($parser->getTriples(), $namespaces);
}

function _evoc_load__error_handler($errno, $errstr) {
    throw new Exception($errstr);
}

/**
 * Queries a set of triples for classes and properties, and builds
 * an associative array describing the vocabulary and any 
 * classes and properties found.
 *
 * @param array $model An ARC2-style array of triples an RDFS vocabulary or OWL ontology
 * @param array $namespaces Associative array of namespaces parsed from the RDF file
 * @param string $ns_prefix Namespace prefix for the vocabulary
 * @param string $ns_uri Only terms in this namespace will be considered
 * @return array Array describing the vocabulary, its classes and properties.
 */
function _evoc_extract_schema(&$model, $namespaces, $ns_prefix, $ns_uri) {
  $terms = _evoc_fetch_terms($model);
  $vocabulary = array(
      'uri' => $ns_uri,
      'title' => _evoc_query_for_title($model, $ns_uri),
      'description' => _evoc_query_for_description($model, $ns_uri),
      'terms' => $terms,
      'namespaces' => $namespaces,
  );

  return $vocabulary;
}

function _evoc_query_for_title(&$model, $ns_uri) {
  $queries = array(
    array($ns_uri, 'http://www.w3.org/2000/01/rdf-schema#label', '?'),
    array($ns_uri, 'http://purl.org/dc/elements/1.1/title', '?'),
    array($ns_uri, 'http://purl.org/dc/terms/title', '?'),
  );
  if (substr($ns_uri, -1) == '#') {
    $uri = substr($ns_uri, 0, -1);
    $queries = array_merge($queries, array(
      array($uri, 'http://www.w3.org/2000/01/rdf-schema#label', '?'),
      array($uri, 'http://purl.org/dc/elements/1.1/title', '?'),
      array($uri, 'http://purl.org/dc/terms/title', '?'),
    ));
  }
  return _evoc_query_find_literal($model, $queries);
}

function _evoc_query_for_description(&$model, $ns_uri) {
  $queries = array(
    array($ns_uri, 'http://www.w3.org/2000/01/rdf-schema#comment', '?'),
    array($ns_uri, 'http://purl.org/dc/elements/1.1/description', '?'),
    array($ns_uri, 'http://purl.org/dc/terms/description', '?'),
  );
  if (substr($ns_uri, -1) == '#') {
    $uri = substr($ns_uri, 0, -1);
    $queries = array_merge($queries, array(
      array($uri, 'http://www.w3.org/2000/01/rdf-schema#comment', '?'),
      array($uri, 'http://purl.org/dc/elements/1.1/description', '?'),
      array($uri, 'http://purl.org/dc/terms/description', '?'),
    ));
  }
  return _evoc_query_find_literal($model, $queries);
}

function _evoc_fetch_terms(&$model) {
  $terms = array();
  $term_uris = array();
  $query_x = array();
  
  // Retrieve the queries for term retrieval. This may have been modified by
  // other modules.
  $term_type_groups = rdfx_term_types();

  foreach($term_type_groups as $term_type_group => $group) {
    foreach ($group['term_types'] as $term_type => $metadata) {
      $query = array(
        array('?', 'http://www.w3.org/1999/02/22-rdf-syntax-ns#type', $metadata['uri']),
        array('?', $metadata['inference']['subject'], null),
        array(null, $metadata['inference']['object'], '?'),
      );
      $term_uris[$term_type] = _evoc_query_find_uris($model, $query);
      
      // Add term details and various relationships for each term, as defined
      // in rdfx_term_types() and altered by hook_rdfx_term_types_alter().
      foreach ($term_uris[$term_type] as $term_uri) {
        $terms[$term_type][$term_uri] = _evoc_query_for_term_description(&$model, $term_uri);
        foreach ($group['description'] as $property => $queries) {
          foreach ($queries['subject'] as $predicate) {
            $query_x[$term_uri][$property][] = array($term_uri, $predicate, '?');
          }
          $terms[$term_type][$term_uri][$property] = _evoc_query_find_uris($model, $query_x[$term_uri][$property]);
        }
      }
    }
  }
  return $terms;
}

function _evoc_query_for_property_uris(&$model) {
  // @todo Move all of the additional classes and property inferences to the
  // rdfx_term_types array and delete this function.
  
  /*return _evoc_query_find_uris($model, array(
    //array('?', 'http://www.w3.org/1999/02/22-rdf-syntax-ns#type', 'http://www.w3.org/1999/02/22-rdf-syntax-ns#Property'),
    //array('?', 'http://www.w3.org/1999/02/22-rdf-syntax-ns#type', 'http://www.w3.org/2002/07/owl#DatatypeProperty'),
    //array('?', 'http://www.w3.org/1999/02/22-rdf-syntax-ns#type', 'http://www.w3.org/2002/07/owl#ObjectProperty'),
    //array('?', 'http://www.w3.org/1999/02/22-rdf-syntax-ns#type', 'http://www.w3.org/2002/07/owl#FunctionalProperty'),
    array('?', 'http://www.w3.org/1999/02/22-rdf-syntax-ns#type', 'http://www.w3.org/2002/07/owl#InverseFunctionalProperty'),
    array('?', 'http://www.w3.org/1999/02/22-rdf-syntax-ns#type', 'http://www.w3.org/2002/07/owl#SymmetricProperty'),
    array('?', 'http://www.w3.org/1999/02/22-rdf-syntax-ns#type', 'http://www.w3.org/2002/07/owl#AnnotationProperty'),
    array('?', 'http://www.w3.org/1999/02/22-rdf-syntax-ns#type', 'http://www.w3.org/2002/07/owl#ReflexiveProperty'),
    array('?', 'http://www.w3.org/1999/02/22-rdf-syntax-ns#type', 'http://www.w3.org/2002/07/owl#IrreflexiveProperty'),
    array('?', 'http://www.w3.org/1999/02/22-rdf-syntax-ns#type', 'http://www.w3.org/2002/07/owl#AsymmetricProperty'),
    array('?', 'http://www.w3.org/1999/02/22-rdf-syntax-ns#type', 'http://www.w3.org/2002/07/owl#TransitiveProperty'),
    array('?', 'http://www.w3.org/2000/01/rdf-schema#domain', null),
    array('?', 'http://www.w3.org/2000/01/rdf-schema#range', null),
    array('?', 'http://www.w3.org/2000/01/rdf-schema#subPropertyOf', null),
    array(null, 'http://www.w3.org/2000/01/rdf-schema#subPropertyOf', '?'),
    array('?', 'http://www.w3.org/2002/07/owl#equivalentProperty', null),
    array(null, 'http://www.w3.org/2002/07/owl#equivalentProperty', '?'),
    array('?', 'http://www.w3.org/2002/07/owl#inverseOf', null),
    array(null, 'http://www.w3.org/2002/07/owl#inverseOf', '?'),
  ));*/
}

function _evoc_query_for_term_description(&$model, $uri) {
  $label = _evoc_query_find_literal($model, array(
    array($uri, 'http://www.w3.org/2000/01/rdf-schema#label', '?')
  ));

  $comment = _evoc_query_find_literal($model, array(
    array($uri, 'http://www.w3.org/2004/02/skos/core#definition', '?'),
    array($uri, 'http://www.w3.org/2000/01/rdf-schema#comment', '?'),
  ));

  //$qname = _evoc_uri_to_qname($uri);
  ////print_r('<br>Term Descrip' . $uri . '<br>');
  return array(
      'uri' => $uri,
      'label' => $label,
      'comment' => $comment,
  );
}

function _evoc_query_for_class_description(&$model, $uri) {
  $class = _evoc_query_for_term_description($model, $uri);
  $class['superclasses'] = _evoc_query_find_qnames($model, array(
    array($uri, 'http://www.w3.org/2000/01/rdf-schema#subClassOf', '?')
  ));
  $class['disjoints'] = _evoc_query_find_qnames($model, array(
    array($uri, 'http://www.w3.org/2002/07/owl#disjointWith', '?'),
    array('?', 'http://www.w3.org/2002/07/owl#disjointWith', $uri),
  ));
  return $class;
}

function _evoc_query_for_property_description(&$model, $uri) {
  ////print_r('<br>' . $uri . '<br>');
  $property = _evoc_query_for_term_description($model, $uri);
  ////print_r('<br>' . $uri . '<br>');
  $property['domains'] = _evoc_query_find_qnames($model, array(
    array($uri, 'http://www.w3.org/2000/01/rdf-schema#domain', '?')
  ));
  $property['ranges'] = _evoc_query_find_qnames($model, array(
    array($uri, 'http://www.w3.org/2000/01/rdf-schema#range', '?')
  ));
  $property['superproperties'] = _evoc_query_find_qnames($model, array(
    array($uri, 'http://www.w3.org/2000/01/rdf-schema#subPropertyOf', '?')
  ));
  $property['is_functional'] = _evoc_query_ask($model, array(
    array($uri, 'http://www.w3.org/1999/02/22-rdf-syntax-ns#type', 'http://www.w3.org/2002/07/owl#FunctionalProperty')
  ));
  $property['is_inverse_functional'] = _evoc_query_ask($model, array(
    array($uri, 'http://www.w3.org/1999/02/22-rdf-syntax-ns#type', 'http://www.w3.org/2002/07/owl#InverseFunctionalProperty')
  ));
  $property['inverses'] = _evoc_query_find_qnames($model, array(
    array($uri, 'http://www.w3.org/2002/07/owl#inverseOf', '?'),
    array('?', 'http://www.w3.org/2002/07/owl#inverseOf', $uri),
  ));
  //print_r($property);

  // @todo TODO We don't handle multiple inverses correctly everywhere yet, so store it both as an array and singleton
  $property['inverse'] = count($property['inverses']) ? $property['inverses'][0] : null;
  return $property;
}

function _evoc_query_ask(&$model, $queries) {
  foreach ($queries as $query) {
    list($s, $p, $o) = $query;
    if (_evoc_query_find_first($model, $s, $p, $o)) return true;
  }
  return false;
}

function _evoc_query_find_literal(&$model, $queries) {
  foreach ($queries as $query) {
    list($s, $p, $o) = $query;
    $triples = _evoc_query_find_all($model, $s, $p, $o);
    // We create an associative array based on the language code of the
    // literal. The language codes Drupal uses are specified in iso.inc.
    foreach ($triples as $triple) {
      if ($triple['o_lang'] !== '') {
        // Chinese and Portuguese are the only languages with a >2 letter
        // langcode.
        if (preg_match('/(zh-hans)|(zh-hant)|(pt-pt)|(pt-br)/', $triple['o_lang'])) {
          $langcode = $triple['o_lang'];
        }
        // Remove the region code if it is used (i.e. en-US is changed to en).
        else {
          $lang_array = explode('-', $triple['o_lang']);
          $langcode = !empty($lang_array[0]) ? $lang_array[0] : $triple['o_lang'];
        }
      }
      else {
        $langcode = 'und';
      }
      $literal[$langcode] = $triple['o'];
    }
  }
  return $literal;
}

function _evoc_query_find_uris(&$model, $queries) {
  $uris = array();
  foreach ($queries as $query) {
    list($s, $p, $o) = $query;
    $result = _evoc_query_find_all($model, $s, $p, $o);
    foreach ($result as $triple) {
      if ($s == '?' && $triple['s_type'] == 'uri') {
        $uris[] = $triple['s'];
      }
      if ($p == '?') {
        $uris[] = $triple['p'];
      }
      if ($o == '?' && $triple['o_type'] == 'uri') {
        $uris[] = $triple['o'];
      }
    }
  }
  return array_unique($uris);
}

function _evoc_query_find_qnames(&$model, $queries) {
  $uris = _evoc_query_find_uris($model, $queries);
  $qnames = array();
  foreach ($uris as $uri) {
    $qnames[] = $uri;
  }
  return $qnames;
}

function _evoc_query_find_first(&$model, $s, $p, $o) {
  foreach ($model as $triple) {
    if (!is_null($s) && $s != '?' && ($triple['s'] != $s || $triple['s_type'] != 'uri')) continue;
    if (!is_null($p) && $p != '?' && ($triple['p'] != $p)) continue;
    if (!is_null($o) && $o != '?' && ($triple['o'] != $o || $triple['o_type'] != 'uri')) continue;
    return $triple;
  }
  return null;
}

function _evoc_query_find_all(&$model, $s, $p, $o) {
  $result = array();
  foreach ($model as $triple) {
    if (!is_null($s) && $s != '?' && ($triple['s'] != $s)) continue;
    if (!is_null($p) && $p != '?' && ($triple['p'] != $p)) continue;
    if (!is_null($o) && $o != '?' && ($triple['o'] != $o)) continue;
    $result[] = $triple;
  }
  return $result;
}