<?php

class Ground_Database {

  public $connection;

  function connect($name) {
    global $db_url;

    if (is_array($db_url)) {
      $config = array_key_exists($name, $db_url) ? $db_url[$name] : $db_url['default'];
    }
    else {
      $config = $db_url;
    }

    $connection_string = $this->create_connection_string($config);
    $this->connection = new PDO($connection_string, $config['username'], $config['password'], array(
                PDO::ATTR_PERSISTENT => true,
                PDO::ATTR_ERRMODE => PDO::ERRMODE_EXCEPTION,
            ));
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
//      if ($property->Null == 'NO') {
//        $sql .= ' NOT NULL';
//      }
//      else if ($property->Default === null) {
//        $sql .= " DEFAULT NULL";
//      }
//
//      if ($property->Default !== null)
//        $sql .= " DEFAULT '$property->Default'";
//
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
    $sql = 'CREATE TABLE IF NOT EXISTS `' . $trellis->get_plural() . "` (\n";
    $sql .= implode(",\n", $fields) . "\n";

//    $keys = db_query('SHOW INDEX FROM ' . $this->name);
//    while ($key = db_fetch_object($keys)) {
//      $this->keys[$key->Key_name][] = $key;
//    }
//
//    foreach ($this->keys as $name => $key) {
//      if ($name == 'PRIMARY') {
//        $column_name = $key[0]->Column_name;
//        $sql .= "PRIMARY KEY (`$column_name`),\n";
//      }
//      else {
//        if (!$key[0]->Non_unique)
//          $sql .= 'UNIQUE ';
//
//        $items = '';
//
//        foreach ($key as $item) {
//          if ($items != '')
//            $items .= ',';
//
//          $items .= "`$item->Column_name`";
//          if ($item->Sub_part != null)
//            $items .= "($item->Sub_part)";
//        }
//
//        $sql .= "KEY `$name` ($items),\n";
//      }
//    }
//    if (count($this->fields) > 0)
//      $sql = substr($sql, 0, strlen($sql) - 2);
//    if ($keystring > 0)
//      $info = db_fetch_object(db_query("SHOW TABLE STATUS WHERE Name = '$this->name'"));
//    $sql .= "\n) ENGINE=$info->Engine";
//    if ($info->Auto_increment) {
//      $sql .= ' AUTO_INCREMENT=' . $info->Auto_increment;
//    }

    $sql .= ");\n";
    
    $this->connection->exec($sql);
  }

  function create_tables($trellises) {
    foreach($trellises as $trellis) {
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

  function query($sql, $parameters = null) {
    $query = $this->connection->prepare($sql);
    $query->execute($parameters);
    return $query;
  }

  function query_array($sql, $parameters = null) {
    $query = $this->query($sql, $parameters);
    return $query->fetchAll();
  }

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

  function query_value($sql) {
    $query = $this->connection->prepare($sql);
    $query->execute();
    return $query->fetchColumn(0);
  }

  function last_insert_id($name) {
    return $this->connection->lastInsertId($name);
  }

}
