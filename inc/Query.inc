<?php

class Query {

  protected $trellises = array();
  public $main_table = 'node';
  public $joins = array();
  public $filters = array();
  public $post_clauses = array();
  public $limit = '';
  public $sources = array();
  public $links = array();
  public $trellis;
  public $db;
  public $include_links = false;
  public $base_path;
  static public $log_queries = false;

  public function __construct($trellis, $include_links = true, $base_path = '') {
    if (get_class($trellis) != 'Trellis' && !is_subclass_of($trellis, 'Trellis')) {
      $x = $x;
      throw new Exception('An invalid trellis was passed to the Query constructor.');
    }
    $this->trellis = $trellis;
    $this->ground = $this->trellis->ground;
    $this->db = $this->ground->db;
    $this->main_table = $trellis->get_table_name();
    $this->base_path = $base_path;
    $this->add_source($trellis);
    if (is_array($include_links))
      $this->include_links = $include_links;

    $current_trellis = $trellis;
    do {
      $links = $current_trellis->get_links();
      foreach ($links as $link) {
        if (is_array($include_links)) {
          if (in_array($link->name, $include_links))
            $this->add_link($link);
        }
        else {
          $path = $this->get_path($link->name);
          if ($include_links || $this->has_expansion($path)) {
            $this->add_link($link);
          }
        }
      }
    }
    while ($current_trellis = $current_trellis->parent);
  }

  function add_source($source, $include_primary_key = true) {
    $this->sources[] = $source;
    $table_name = $source->get_table_name();
    $core_properties = $source->get_core_properties();
    foreach ($core_properties as $property) {
      if ($property->name != $source->primary_key || $include_primary_key) {
        $field_name = $property->get_field_name();
        $sql = $table_name . '.' . $field_name;
        if ($field_name != $property->name)
          $sql .= " AS  `$property->name`";
        $this->add_field($sql);
      }
    }

    $source->parent_query($this);
  }

  function add_filter($clause) {
    $this->filters[] = $clause;
  }

  function add_field($clause) {
    $this->fields[] = $clause;
  }

  function add_join($clause) {
    $this->joins[] = $clause;
  }

  function add_post($clause) {
    $this->post_clauses[] = $clause;
  }

  function generate_pager($offset = 0, $limit = 0) {
    if ($offset == 0) {
      if ($limit == 0)
        return '';
      else
        return " () LIMIT $limit";
    }
    else {
      if ($limit == 0) {
        $limit = 18446744073709551615;
      }

      return " LIMIT $offset, $limit";
    }
  }

  function add_link($property) {
    if (is_string($property)) {
      $name = $property;
      $property = $this->trellis->properties[$name];
      if (!$property)
        throw new Exception($this->trellis->name . ' does not have a property named ' . $name . '.');
    }

    if (array_key_exists($property->name, $this->links)) {
      throw new Exception("$property->name added twice to query!");
    }

    $link = new stdClass();
    $link->other = $property->get_referenced_trellis();
    $link->property = $property;
    $this->links[$property->name] = $link;

//    if ($property->type == 'reference') {
//      $this->add_field($property->get_field_name() . " AS `$property->name`");
//    }
  }

  function add_links($paths) {
    foreach ($paths as $path) {
      $this->add_link($path);
    }
  }

  function add_pager() {
    $this->limit = $this->generate_pager((int) $_GET['offset'], (int) $_GET_['limit']);
  }

  function paged_sql($sql) {
    if ($this->limit != '')
      $sql .= ' ' . $this->limit;

    return $sql;
  }

  function remove_field($table, $field_name) {
    if ($this->trellises[$table])
      unset($this->trellises[$table]->fields[$field_name]);
  }

  function generate_sql() {
    global $user;

    $sql = 'SELECT ';
    $sql .= implode(",\n", $this->fields);

    $sql .= "\nFROM " . $this->main_table;

    if (count($this->joins) > 0) {
      $sql .= "\n" . implode("\n", $this->joins);
    }

    if (count($this->filters) > 0) {
      $sql .= "\nWHERE " . implode(' AND ', $this->filters);
    }

    if (count($this->post_clauses) > 0) {
      $sql .= ' ' . implode(' ', $this->post_clauses);
    }

    return $sql;
  }

  function run() {
    $result = new stdClass();
    $result->objects = array();
    $sql = $this->generate_sql();
    $sql = str_replace("\r", "\n", $sql);
    $paged_sql = $this->paged_sql($sql);
    if (Query::$log_queries)
      echo $sql . "\n\n";

    $rows = $this->db->query_objects($paged_sql);
    foreach ($rows as $row) {
      $this->process_row($row);
      $result->objects[] = $row;
    }

    $this->post_process_result($result);
    return $result->objects;
  }

  function run_as_service($return_sql = false) {
    $result = new stdClass();
    $result->objects = array();
    $sql = $this->generate_sql();
    $sql = str_replace("\r", "\n", $sql);
    $paged_sql = $this->paged_sql($sql);

    $this->sql = $paged_sql;
    $rows = $this->db->query_objects($paged_sql);
    foreach ($rows as $row) {
      $this->process_row($row);
      $result->objects[] = $row;
    }

    $this->post_process_result($result);

    if ($return_sql)
      $result->sql = $this->sql;

    return $result;
  }

  function process_row(&$row) {
    // Map field names to bloom property names.
    foreach ($this->trellis->properties as $property) {
      $field_name = $property->get_field_name();
      if ($property->name != $field_name) {
        if (isset($row->{$field_name})) {
          $row->{$property->name} = $row->{$field_name};
          unset($row->{$field_name});
        }
      }
    }

    foreach ($this->trellises as $item) {
      $this->trellises[$item->name]->translate($row);
    }

    foreach ($this->sources as $source) {
      foreach ($source->properties as $property) {
        $full_name = $property->name;

        if (property_exists($row, $full_name)) {
          $row->$full_name = $this->ground->convert_value($row->$full_name, $property->type);
        }
      }
    }

    foreach ($this->links as $name => $link) {
      $property = $link->property;
      $id = $row->{$property->parent->primary_key};

      switch ($property->get_relationship()) {
        case RELATIONSHIP_ONE_TO_ONE:
          $row->{$name} = $this->get_reference_object($row, $property, $link->other);
          break;
        case RELATIONSHIP_ONE_TO_MANY:
          $row->{$name} = $this->get_one_to_many_list($id, $property, $link->other);
          break;
        case RELATIONSHIP_MANY_TO_MANY:
          $row->{$name} = $this->get_many_to_many_list($id, $property, $link->other);
          break;
      }
    }
  }

  function get_many_to_many_list($id, $property, $other_table) {
    $other_property = $property->get_other_property();
    $query = $this->ground->create_query($other_table, false, $this->get_path($this->trellis->name, $property->name));
    $link_class = $property->get_link_class();
    $join = new $link_class($property);
    $join_sql = $join->generate_join($id);
    $query->add_join($join_sql);
    return $query->run();
  }

  function get_one_to_many_list($id, $property, $other_table) {
    $other_property = $property->get_other_property(false);
    if (!$other_property) {
      $other_property = $other_table->properties[$other_table->primary_key];
    }

    $query = $this->ground->create_query($other_table, $this->include_links, $this->get_path($this->trellis->name, $property->name));
    $query->add_filter($other_property->query() . ' = ' . $id);
    return $query->run();
  }

  function get_reference_object($row, $property, $other_table) {
    $query = $this->ground->create_query($other_table, false, $this->get_path($this->trellis->name, $property->name));
    if (!isset($row->{$property->name})) {
      return null;
//      throw new Exception("Query result does not have $property->name.");
    }
    $query->add_filter($other_table->query_primary_key() . ' = ' . $row->{$property->name});
    $objects = $query->run();
    return $objects[0];
  }

  function get_path($first = null, $second = null) {
    $items = array();
    if ($this->base_path)
      $items[] = $this->base_path;

    if ($first)
      $items[] = $first;

    if ($second)
      $items[] = $second;

    return implode('/', $items);
  }

  function has_expansion($path) {
//    if (!$this->base_path)
//      return false;
    //$path = $this->get_path($property->name);
    foreach ($this->ground->expansions as $expansion) {
      if (substr($expansion, 0, 1) == '/' && substr($expansion, strlen($expansion) - 1, 1) == '/') {
        return preg_match($expansion, $path) == 1;
      }
      else {
        return $path == $expansion;
      }
    }
    return false;
  }

  function post_process_result($result) {
    
  }

}

