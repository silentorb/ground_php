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

  public function __construct($trellis, $include_links = true) {
    if (get_class($trellis) != 'Trellis' && !is_subclass_of($trellis, 'Trellis')) {
      $x = $x;
      throw new Exception('An invalid trellis was passed to the Query constructor.');
    }
    $this->trellis = $trellis;
    $this->ground = $this->trellis->ground;
    $this->db = $this->ground->db;
    $this->main_table = $trellis->get_table_name();
    $this->add_source($trellis);
    if ($include_links) {
      $current_trellis = $trellis;
      do {
        foreach ($current_trellis->links as $link) {
          $this->add_link($link);
        }
      }
      while ($current_trellis = $current_trellis->parent);
    }
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
          $sql .= ' AS ' . $property->name;
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

    if ($property->type == 'reference') {
      $this->add_field($property->get_field_name() . " AS `$property->name`");
    }
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
//    echo $sql . "\n\n";
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
          $row->$full_name = Ground::convert_value($row->$full_name, $property->type);
        }
      }
    }

    foreach ($this->links as $name => $link) {
      $property = $link->property;
      $id = $row->{$property->parent->primary_key};
      $other_property = $link->other->get_link_property($property->parent);
      if ($other_property === null)
        throw new Exception($property->parent->name . '->' . $property->name . ' does not have a reciprocal reference on ' . $link->other->name . '.');

      if ($property->type == 'list' && $other_property->type == 'list') {
        // Many to Many
        $row->{$name} = $this->get_many_to_many_list($id, $property, $other_property, $link->other);
      }
      else if ($property->type == 'list') {
        // One to Many
        $row->{$name} = $this->get_one_to_many_list($id, $property, $other_property, $link->other);
      }
      else {
        // One to One
        $row->{$name} = $this->get_reference_object($row, $property, $link->other);
      }
    }
  }

  function get_many_to_many_list($id, $property, $other_property, $other_table) {
    $query = $this->ground->create_query($other_table, false);
    $link_class = $property->get_link_class();
    $join = new $link_class($property);
    $join_sql = $join->generate_join($id);
    $query->add_join($join_sql);
    return $query->run();
  }

  function get_one_to_many_list($id, $property, $other_property, $other_table) {
    $query = $this->ground->create_query($other_table, false);
    $query->add_filter($other_property->query() . ' = ' . $id);
    return $query->run();
  }

  function get_reference_object($row, $property, $other_table) {
    $query = $this->ground->create_query($other_table, false);
    if (!isset($row->{$property->name})) {
      return null;
//      throw new Exception("Query result does not have $property->name.");
    }
    $query->add_filter($other_table->query_primary_key() . ' = ' . $row->{$property->name});
    $objects = $query->run();
    return $objects[0];
  }

  function post_process_result($result) {
    
  }

}
