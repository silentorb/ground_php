<?php

require_once dirname(__FILE__) . DIRECTORY_SEPARATOR . 'Path.inc';

interface IChannel {
  public function run($request);
}

class Channel implements IChannel {

  public $path;
  public $target;
  public $method;
  public $walls = array();
  public $ground;

  public function __construct($path, $target, $method) {
    if (is_string($path))
      $this->path = Irrigation::get_path_array($path, Irrigation::$base_path);
    else {
      $this->path = $path;
    }

    $this->target = $target;
    $this->method = $method;
  }

  public function run($request) {
    return $this->target->{$this->method}($request);
  }

}

class Bag {

  public $path;
  public $arguments = array();
  public $objects;
  public $resources = array();
  public $GET = array();
  public $POST = array();

  public function __construct($path = null) {
    if (is_string($path))
      $this->path = Irrigation::get_path_array($path, Irrigation::$base_path);
    else if (is_array($path))
      $this->path = $path;
    else
      $this->path = array();
  }

}

class Irrigation {

  public static $base_path = 'vineyard';
  public $channels = array();
  // Not used yet.  Will be used for optimized routing.
  public $tree = array();
  // Ground is optional.
  public $ground;

  public function __construct($ground = null) {
    $this->ground = $ground;
  }

  public function add_channel($path, $target, $method, $walls = null) {
    $channel = new Channel($path, $target, $method);
    $channel->ground = $this->ground;
    $this->channels[] = $channel;

    if (is_array($walls)) {
      $channel->walls = $walls;
      if (isset($channel->ground) && isset($channel->ground->fortress)) {
        foreach ($channel->walls as $wall) {
          $wall->fortress = $channel->ground->fortress;
        }
      }
    }
  }

  public function generate_tree() {
    // Later
  }

  public function compare($pattern, $text, $request = null) {
    if (!$pattern || !$text)
      return false;

    if ($pattern[0] == '%') {
      $key = substr($pattern, 1);
      if (isset($this->ground->trellises[$key])) {
        $query = $this->ground->create_query($key);
        $query->add_key_filter($text);
        $object = $query->run_single();
        if (!$object)
          return false;

        if (is_object($request))
          $request->resources[] = $object;
      }

      return true;
    }

    if ($pattern == '@trellis') {
      if ($this->ground)
        return array_key_exists($text, $this->ground->trellises);
      else
        return false; // Should an exception be thrown here?
    }

    return $pattern == $text;
  }

  public function process($path, $request) {
    $channel = $this->find_channel($path, $request);

    if (!$channel) {
      throw new Exception('Path not found', 404);
    }

    $channel->run($request);
  }

  public function find_channel($path, $request = null) {
    if (is_string($path))
      $path = Irrigation::get_path_array($path, Irrigation::$base_path);

    // We're going to utilize PHP's copying arrays by value.
    $channels = array();
    foreach ($this->channels as $channel) {
      if (count($channel->path) == count($path))
        $channels[] = $channel;
    }

    $x = 0;
    foreach ($path as $step) {
      $next_channels = array();
      foreach ($channels as $channel) {
        if ($this->compare($channel->path[$x], $step, $request))
          $next_channels[] = $channel;
      }
      $channels = $next_channels;
      ++$x;
    }

    return $channels[0];
  }

  static function create_bag_from_globals() {
    $bag = new Bag();

    // In case the current website is inside a subpath:
    $script_name = dirname($_SERVER['SCRIPT_NAME']);
    if (strlen($script_name) > 1) {
      $base = $script_name . '/' . Irrigation::$base_path;
    } else {
      $base = Irrigation::$base_path;
    }
    $bag->path = Irrigation::get_path_array(Irrigation::get_simple_url_path($_SERVER["REQUEST_URI"]), $base);
    $bag->arguments = $_REQUEST;
    $bag->GET = $_GET;
    if (count($_POST) > 0) {
      $bag->POST = $_POST;
    } else {
      $json = file_get_contents("php://input");
      $data = json_decode(utf8_encode($json));
      if ($data)
        $bag->POST = (array)$data;

      if (isset($bag->POST['objects'])) {
        $objects = $bag->POST['objects'];
      }
    }

    return $bag;
  }

  static function convert_path_to_array($path) {
    if (!$path || strlen($path) == 0)
      return array();

    if ($path[0] == '/')
      $path = substr($path, 1);

    if (substr($path, strlen($path) - 1) == '/')
      $path = substr($path, 0, strlen($path) - 1);

    return explode('/', $path);
  }

  static function get_path_array($path, $base) {
    $path = Irrigation::convert_path_to_array($path);
    $base = Irrigation::convert_path_to_array($base);

    $amount = 0;
    foreach ($base as $i => $item) {
      if ($item == $path[$i]) {
        $amount++;
      } else {
        break;
      }
    }

    if ($amount)
      array_splice($path, 0, $amount);

    return $path;
  }

  static function get_simple_url_path($url) {
    $index = strpos($url, '?');
    if ($index !== false) {
      return substr($url, 0, $index);
    } else {
      return $url;
    }
  }

  static function path_arrays_match($a, $b) {
    if (count($a) != count($b))
      return false;

    foreach ($a as $key => $value) {
      if ($b[$key] != $value)
        return false;
    }

    return true;
  }

  static function import() {

  }

  public function load_api_from_file($schema_file) {
    $data = Ground::load_json_from_file($schema_file);
    $this->parse_api($data);
  }

  public function parse_api($data) {
    if (isset($data->channels)) {
      $this->load_channels($data->channels);
    }
  }

  public function load_channels($channels) {
    foreach ($channels as $path => $data) {
      $this->channels[] = new API_Channel($data->target, $data->method, $data->properties);
    }
  }

  static function prepare_input($parameters, $arguments) {
    $result = array();
    foreach ($parameters as $name => $parameter) {
      $parameter = (object)$parameter;
      if (isset($arguments[$name])) {
        $result[$name] = Irrigation::get_value($arguments[$name], $parameter->type);
      } else if (isset($parameter->default)) {
        $result[$name] = $parameter->default;
      }
    }
    return $result;
  }

  // Similar to the Ground version but simpler and not extendable.  (Which means less dependancies.)
  static function get_value($value, $type) {
    switch ($type) {
      case 'int':
        return (int)$value;
        break;
      case 'string':
      case 'text':
      case 'reference':
        return $value;
        break;
      case 'bool':
        return str_to_bool($value);
        break;
      case 'float':
      case 'double':
        return (float)$value;
        break;
    }

    return null;
  }

  public function service($request) {
    return $this->ground->vineyard_service($request);
  }

}

//class API_Channel implements IChannel {
//
//  public $target;
//  public $method;
//  public $parameters;
//
//  public function __construct($target, $method, $parameters) {
//    $this->target = $target;
//    $this->method = $method;
//    $this->parameters = $parameters;
//  }
//
//  public function run($request) {
//    $input = Irrigation::prepare_input($this->parameters, $request);
//    return $this->target->{$this->method}($input, $request);
//  }
//
//}