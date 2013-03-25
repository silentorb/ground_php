<?php

module_load_include('inc', 'ground_drupal', 'inc/Query');

class Content_Query extends \ground\Query {
  function run() {
    $objects = array();
    $properties = $this->trellis->get_all_properties();
    $sql = $this->generate_sql($properties);
    $sql = str_replace("\r", "\n", $sql);
    $paged_sql = $this->paged_sql($sql);

    $rows = $this->db->query_objects($paged_sql);
    foreach ($rows as $row) {
      $query = $this->ground->create_query($row->type);
//      $query->filters = $this->filters;
      $query->add_filter("node.nid = $row->nid");
      $query->add_filter("content_type_$row->type.vid = $row->vid");
      $nodes = $query->run();
      if (count($nodes) > 0) {
        $nodes[0]->trellis = $row->type;
        $objects[] = $nodes[0];
      }
    }

    return $objects;
  }

}
