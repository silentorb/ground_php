<?php

namespace ground;

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
  public $include_links = true;
  public $fields = array();
  public $base_path;
  public $arguments = array();
  public $properties = null; // Set this to an array to only use the properties in this list.
  public $definition = null; // Structured definition if any was used to define this query
  static public $log_queries = false;

  static $operators = array('=', 'LIKE', '!=', 'is');

  public function __construct($trellis, $base_path = null) {
    if (get_class($trellis) != 'Trellis' && !is_subclass_of($trellis, 'Trellis')) {
      throw new \Exception('An invalid trellis was passed to the Query constructor.');
    }
    $this->trellis = $trellis;
    $this->ground = $this->trellis->ground;
    $this->expansions = $this->ground->expansions;
    $this->db = $this->ground->db;
    $this->main_table = $trellis->get_table_name();
    if ($base_path)
      $this->base_path = $base_path;
    else
      $this->base_path = $this->trellis->name;

    $this->ground->invoke('construct.query.' . $trellis->name, $this);
  }

  function add_arguments($arguments) {
    $this->arguments = array_merge($this->arguments, $arguments);
  }

  function add_filter($clause, $arguments = null) {
    $this->filters[] = $clause;
    if ($arguments) {
      $this->add_arguments($arguments);
    }
  }

  // LIKE is not currently supported for JOIN filters
  function add_property_filter($property, $value = null, $operator = '=') {
    if (!in_array($operator, Query::$operators))
      throw new \Exception("Operator is not supported: $operator.");

    if (is_string($property)) {
      $properties = $this->trellis->get_all_properties();
      if (!isset($properties[$property])) {
        throw new \Exception($this->trellis->name . ' does not contain a property named ' . $property . '.');
      }
      $property = $properties[$property];
    }

    $placeholder = ':' . $property->name . '_filter';
    if ($value === 'null' && $this->type != 'string') {
      $this->filters[] = $property->query() . ' IS NULL';
      return;
    }

    if ($value !== null)
      $value = $this->ground->convert_value($value, $property->type);

    if ($property->get_relationship() == RELATIONSHIP_MANY_TO_MANY) {
      $this->add_property_join($property, $placeholder, true);
    }
    else {
      if ($operator == 'LIKE') {
        $this->filters[] = $property->query() . ' LIKE ' . $placeholder;
        if ($value !== null)
          $value = '%' . (string)$value . '%';
      }
      else {
        $this->filters[] = $property->query() . " $operator $placeholder";
      }
    }

    if ($value !== null)
      $this->add_arguments(array($placeholder => $value));
  }

  function add_key_filter($value) {
    $this->filters[] = $this->trellis->query_primary_key() . ' = :primary_key';
    $this->add_arguments(array(':primary_key' => $value));
  }

  function add_field($clause, $arguments = null) {
    $this->fields[] = $clause;
    if ($arguments) {
      $this->add_arguments($arguments);
    }
  }

  function add_join($clause, $arguments = null) {
    $this->joins[] = $clause;
    if ($arguments) {
      $this->add_arguments($arguments);
    }
  }

  function add_property_join($property, $id, $reverse = true) {
    $link_class = $property->get_link_class();
    $join = new $link_class($property);
    $join_sql = $join->generate_join($id, $reverse);
    $this->add_join($join_sql);
  }

  function add_post($clause, $arguments = null) {
    $this->post_clauses[] = $clause;
    if ($arguments) {
      $this->add_arguments($arguments);
    }
  }

  function add_expansion($clause) {
    $this->expansions[] = $clause;
  }

  function generate_pager($offset = 0, $limit = 0) {
    if ($offset == 0) {
      if ($limit == 0)
        return '';
      else
        return " LIMIT $limit";
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
        throw new \Exception($this->trellis->name . ' does not have a property named ' . $name . '.');
    }

    if (array_key_exists($property->name, $this->links)) {
      throw new \Exception("$property->name added twice to query!");
    }

    $link = new \stdClass();
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
    $this->limit = $this->generate_pager((int)$_GET['offset'], (int)$_GET['limit']);
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

  function generate_sql($properties) {
    $data = $this->get_fields_and_joins($properties);
    $fields = array_merge($data['fields'], $this->fields);
    $joins = array_merge($data['joins'], $this->joins);

    if (count($fields) == 0) {
      throw new \Exception("No authorized fields found for trellis $this->main_table.");
    }

    $sql = 'SELECT ';
    $sql .= implode(",\n", $fields);

    $sql .= "\nFROM " . $this->main_table;

    if (count($joins) > 0) {
      $sql .= "\n" . implode("\n", $joins);
    }

    if (count($this->filters) > 0) {
      $sql .= "\nWHERE " . implode(' AND ', $this->filters);
    }

    if (count($this->post_clauses) > 0) {
      $sql .= ' ' . implode(' ', $this->post_clauses);
    }

    return $sql;
  }

  function run($arguments = array()) {
    $result = new \stdClass();
    $result->objects = array();
    if ($this->properties)
      $properties = $this->properties;
    else
      $properties = $this->trellis->get_all_properties();
    
    $sql = $this->generate_sql($properties);
    $sql = str_replace("\r", "\n", $sql);
    $paged_sql = $this->paged_sql($sql);
    if (Query::$log_queries)
      echo $sql . "\n\n";

    $arguments = array_merge($this->arguments, $arguments);
    $rows = $this->db->query_objects($paged_sql, $arguments);
    foreach ($rows as $row) {
      $this->process_row($row, $properties);
      $result->objects[] = $row;
    }

    $this->post_process_result($result);
    return $result->objects;
  }

  function run_single($arguments = array()) {
    $objects = $this->run($arguments);
    return $objects[0];
  }

  function run_as_service($user = null) {
    $result = new \stdClass();
    $result->objects = array();
    \Ground::remove_private_fields_from_array($this->fields, $this->trellis);
    if ($this->properties)
      $properties = $this->properties;
    else
      $properties = $this->trellis->get_all_properties();

    if (is_object($this->ground->fortress))
      $properties = $this->ground->fortress->get_authorized_properties($properties, $this->trellis->name . '/', $user);

    $sql = $this->generate_sql($properties);
    $sql = str_replace("\r", "\n", $sql);
    $paged_sql = $this->paged_sql($sql);

    $prohibit_count = 0;
    $this->sql = $paged_sql;
    $rows = $this->db->query_objects($paged_sql, $this->arguments);
    foreach ($rows as $row) {
      if (is_object($this->ground->fortress)) {
        // User needs an access level of at least 1 to view these objects
        $access = $this->ground->fortress->access(null, $row, array('trellis' => $this->trellis));
        if ($access == 0) {
          ++$prohibit_count;
          continue;
        }
      }
      $this->process_row($row, $properties);
      $result->objects[] = $row;
    }

    $count_sql = 'SELECT COUNT(*) FROM (' . $sql . ') tmp';
    $result->total = ((int)$this->db->query_value($count_sql, $this->arguments)) - $prohibit_count;
    $this->post_process_result($result);

//    if ($return_sql)
//      $result->sql = $this->sql;

    return $result;
  }

  function get_fields_and_joins($properties, $include_primary_key = true) {
    $fields = array();
    $trellises = array();
    if ($this->trellis->name == 'branch') {
      $key = 1;
    }
    foreach ($properties as $property) {
      if ($property->type == 'list' || $property->virtual)
        continue;

      $field = $property->get_field_override();
      // Virtual properties aren't saved to the database.
      // Useful when you define custom serialization hooks
      if ($field->virtual)
        continue;

      if ($property->name != $this->trellis->primary_key || $include_primary_key) {
        $field_name = $property->get_field_name();
//        $table_name = $this->trellis->get_table_name();
        $sql = $property->query();
        if ($field_name != $property->name)
          $sql .= " AS  `$property->name`";
        $fields[] = $sql;
        $trellises[$property->parent->name] = $property->parent;
      }
    }
    $joins = array();
    foreach ($trellises as $trellis) {
      $join = $trellis->get_join($this->main_table);
      if ($join)
        $joins[] = $join;
    }

    return array(
      'fields' => $fields,
      'joins' => $joins,
    );
  }

  function process_row(&$row, $authorized_properties = null) {
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

    if ($authorized_properties) {
      foreach ($authorized_properties as $property) {
        $full_name = $property->name;

        if (property_exists($row, $full_name)) {
          $row->$full_name = $this->ground->convert_value($row->$full_name, $property->type);
        }
      }
    }
//    foreach ($this->sources as $source) {
//      foreach ($source->properties as $property) {
//        $full_name = $property->name;
//
//        if (property_exists($row, $full_name)) {
//          $row->$full_name = $this->ground->convert_value($row->$full_name, $property->type);
//        }
//      }
//    }

    $links = $this->trellis->get_all_links();

    foreach ($links as $name => $property) {
      if ($property->virtual)
        continue;

      $path = $this->get_path($property->name);
      if ($authorized_properties && !array_key_exists($name, $authorized_properties))
        continue;

      if ($this->include_links === true || $this->has_expansion($path)) {
        $other_table = $property->get_referenced_trellis();
        //$property = $link->property;
        $id = $row->{$property->parent->primary_key};

        switch ($property->get_relationship()) {
          case RELATIONSHIP_ONE_TO_ONE:
            $row->{$name} = $this->get_reference_object($row, $property, $other_table);
            break;
          case RELATIONSHIP_ONE_TO_MANY:
            $row->{$name} = $this->get_one_to_many_list($id, $property, $other_table);
            break;
          case RELATIONSHIP_MANY_TO_MANY:
            $row->{$name} = $this->get_many_to_many_list($id, $property, $other_table);
            break;
        }
      }
    }

    $this->ground->invoke('process.row.' . $this->trellis->name, $row, $this, $this->trellis);
  }

  private function create_subquery($trellis, $property) {
    $query = $this->ground->create_query($trellis, $this->get_path($property->name));
    $query->include_links = false;
    $query->expansions = $this->expansions;
    $definition = $this->definition;
    if (is_array($definition) && is_object($definition['properties']->{$property->name})) {
      $this->ground->customize_query_from_post($query, $definition['properties']->{$property->name});
    }

    return $query;
  }

  function get_many_to_many_list($id, $property, $other_table) {
    $other_property = $property->get_other_property();
    $query = $this->create_subquery($other_table, $property);
    $query->add_property_join($property, $id);
    return $query->run();
  }

  function get_one_to_many_list($id, $property, $other_table) {
    $other_property = $property->get_other_property(false);
    if (!$other_property) {
      $other_property = $other_table->properties[$other_table->primary_key];
    }

    $query = $this->create_subquery($other_table, $property);
    $query->add_filter($other_property->query() . ' = ' . $id);
    return $query->run();
  }

  function get_reference_object($row, $property, $other_table) {
    $query = $this->create_subquery($other_table, $property);
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
//    print $path . "\n";
//    if (!$this->base_path)
//      return false;
    //$path = $this->get_path($property->name);
    foreach ($this->expansions as $expansion) {
      if (substr($expansion, 0, 1) == '/' && substr($expansion, strlen($expansion) - 1, 1) == '/') {
        if (preg_match($expansion, $path))
          return true;
      }
      else {
        if ($path == $expansion)
          return true;
      }
    }
    return false;
  }

  function post_process_result($result) {

  }

}
