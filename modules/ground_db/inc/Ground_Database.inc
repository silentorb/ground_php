<?php

class Ground_Database {

  public $connection;

  function connect($name) {
    global $db_url;

    if (is_array($db_url)) {
      if (array_key_exists($name, $db_url))
        $config = $db_url[$name];
      else
        $config = $db_url['default'];
    }
    else {
      $config = $db_url;
    }

    $connection_string = $this->create_connection_string($config);
//    $this->connection = new PDO($connection_string, $config['username'], $config['password'], array(
//                PDO::ATTR_PERSISTENT => true,
//                PDO::ATTR_ERRMODE => PDO::ERRMODE_EXCEPTION,
//            ));
  }

  protected function create_connection_string($config) {
    $driver = $config['driver'];
    $host = $config['host'];
    $database = $config['database'];
    return "$driver:host=$host;dbname=$database";
  }

  function create_table($trellis) {
    $keys = array();
    $fields = array();
    foreach ($trellis->properties as $property) {
      $type = $property->get_field_type();
      if ($type === null)
        continue;

      $field_sql = "`$property->name` $type";
      if ($property->name == $trellis->primary_key) {
        $field_sql .= ' AUTO_INCREMENT';
        $keys[] = "PRIMARY KEY (`$property->name`)\n";
      }

      $fields[] = $field_sql;
    }

    // Can't create a table without fields
    if (count($fields) == 0)
      return;

    $fields = array_merge($fields, $keys);
    $sql = 'CREATE TABLE IF NOT EXISTS `' . $trellis->get_table_name() . "` (\n";
    $sql .= implode(",\n", $fields) . "\n";

    $sql .= ");\n";

    $this->connection->exec($sql);
  }

  function create_tables($trellises) {
    foreach ($trellises as $trellis) {
      $this->create_table($trellis);
    }
  }

  function drop_all_tables() {
    $db = $this->connection;
    $db->query('SET foreign_key_checks = 0');
    $tables = $this->get_tables();
    foreach ($tables as $table) {
      $db->query('DROP TABLE IF EXISTS ' . $table);
    }

    $db->query('SET foreign_key_checks = 1');
  }

  function get_tables() {
    $db = $this->connection;
    $result = array();
    $rows = $db->query("SHOW TABLES");
    foreach ($rows as $row) {
      $result[] = $row[0];
    }

    return $result;
  }

  // Return a raw query for further data extraction
  function query($sql, $parameters = null) {
    $query = $this->connection->prepare($sql);
    try {
      $query->execute($parameters);
    }
    catch (Exception $ex) {
      throw new Exception($ex->getMessage() . " \n" . $sql);
    }
    return $query;
  }

  // Return an array of associative arrays
  function query_array($sql, $parameters = null) {
    $query = $this->query($sql, $parameters);
    return $query->fetchAll();
  }

  // Return an array of objects
  function query_objects($sql, $parameters = null) {
    $query = $this->query($sql, $parameters);

    // This may not be the most optimal way to get an array of objects
    // from PDO, but it functions properly for now and can be discretely
    // optimized later.
    $items = $query->fetchAll();
    $result = array();
    foreach ($items as $item) {
      $result[] = (object) $item;
    }

    return $result;
  }

  // Return a single value
  function query_value($sql, $parameters = null) {
    $query = $this->query($sql, $parameters);
    return $query->fetchColumn(0);
  }

  // Return an array of single values, usually a list of ids or names
  function query_values($sql, $parameters = null) {
    $query = $this->query($sql, $parameters);
  //  return $query->fetchAll(PDO::FETCH_COLUMN, 0);
  }

  function last_insert_id($name) {
    return $this->connection->lastInsertId($name);
  }

}
