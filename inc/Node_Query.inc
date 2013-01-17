<?php

/**
 * Description of Service_Query
 *
 * @author Chris Johnson
 */
class Node_Query extends Dynamic_Query {

  protected $root_object;

  public function __construct() {
//    $this->root_object = new Object_Map('root', array(
//
//            ));

    $this->objects['node'] = new Object_Map('node', array(
                'nid' => 'int',
                'created' => 'int',
                'changed' => 'int',
                'uid' => 'int',
            ));
    
    $this->objects['node']->is_root = true;
    $this->add_join("LEFT JOIN node_revisions ON node.vid = node_revisions.vid\n");
    $this->add_join("LEFT JOIN users ON node.uid = users.uid\n");
    
    $this->add_field('node.title AS title');
    $this->add_field('node_revisions.body AS body');
    $this->add_field('node.type AS type');
    $this->add_field('users.name AS user_name');
  }

  function add_location() {
    if ($this->location_added)
      return;

    $this->objects['location'] = new Object_Map('location', array(
                'latitude' => 'string',
                'longitude' => 'string',
                'name' => 'string',
                'street' => 'string',
                'additional' => 'string',
                'city' => 'string',
                'province' => 'string',
                'postal_code' => 'string',
                'country' => 'string',
                'source' => 'string',
                'is_primary' => 'string',
            ));

    $this->add_location_join();
    $this->location_added = true;
  }

  function add_location_join() {
    if ($this->location_join_added)
      return;

    $this->add_join(<<<SQL
LEFT JOIN location_instance ON location_instance.nid = node.nid
LEFT JOIN location ON location_instance.lid = location.lid

SQL
    );

    $this->location_join_added = true;
  }

  function add_proximity($latitude, $longitude, $distance) {
    $distance_meters = _location_convert_distance_to_meters($distance, 'mile');
    $latrange = earth_latitude_range($longitude, $latitude, $distance_meters);
    $lonrange = earth_longitude_range($latitude, $latitude, $distance_meters);

    // Add MBR check (always).
    // In case we go past the 180/-180 mark for longitude.
    if ($lonrange[0] > $lonrange[1]) {
      $where = "location.latitude > $latrange[0] AND location.latitude < $latrange[1] AND ((location.longitude < 180 AND location.longitude > $lonrange[0]) OR (location.longitude < $lonrange[1] AND location.longitude > -180))";
    }
    else {
      $where = "location.latitude > $latrange[0] AND location.latitude < $latrange[1] AND location.longitude > $lonrange[0] AND location.longitude < $lonrange[1]";
    }
    // Add radius check.
    $this->add_filter(earth_distance_sql($longitude, $latitude, 'location') . ' < ' . $distance_meters);
    $this->add_location();
  }

//  function generate_sql() {
//    global $user;
//
//    $sql = <<<SQL
//SELECT
//
//  -- Simple fields --
//node.title AS title,
//node_revisions.body AS body,
//node.type AS type,
//users.name AS user_name
//
//SQL;
//
//    $fields = array_merge($this->fields, array());
//    foreach ($this->objects as $object) {
//      $snippet = $object->render_fields();
//      if ($snippet)
//        $fields[] = $snippet;
//    }
//    if (count($fields) > 0) {
//      $sql .= ', ';
//    }
//    $sql .= implode(', ', $fields);
//
//    $sql .= <<<SQL
//
//  -- Joins --
//FROM node 
//LEFT JOIN node_revisions ON node.vid = node_revisions.vid
//LEFT JOIN users ON node.uid = users.uid
//
//SQL;
//
//    $sql .= ' ' . implode(' ', $this->joins);
//    if (count($this->filters) > 0) {
//      $sql .= ' WHERE ' . implode(' AND ', $this->filters);
//    }
//    $sql .= ' ' . implode(' ', $this->post_clauses);
//
//    return $sql;
//  }

}
