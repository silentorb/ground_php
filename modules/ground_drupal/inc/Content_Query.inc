<?php

class Content_Query extends Query {
  function run() {
    $objects = array();
    $sql = $this->generate_sql();
    $sql = str_replace("\r", "\n", $sql);
    $paged_sql = $this->paged_sql($sql);
    if (Query::$log_queries)
      echo $sql . "\n\n";

    $rows = $this->db->query_objects($paged_sql);
    foreach ($rows as $row) {
      $query = $this->ground->create_query($row->type);
//      $query->filters = $this->filters;
      $query->add_filter("node.nid = $row->nid");
      $nodes = $query->run();
      if (count($nodes) > 0) {
        $nodes[0]->trellis = $row->type;
        $objects[] = $nodes[0];
      }
    }

    return $objects;
  }

}
