<?php

class Ground_Module extends Meta_Object
{

    public $ground;

    public function __construct()
    {
        $this->listen($this, 'connect.ground', 'initialize');
    }

    public function initialize($ground)
    {
        $this->ground = $ground;
        
        // I don't have time to write tests for this yet
        // so I'm leaving this commented for now.
/*
        $info = new ReflectionObject($this);
        $path = dirname($info->getFileName());
        $path = str_replace("\\", '/', $path);
        $json_file = dirname($path . '../json') . '/' . strtolower($info->getName()) . '.json';
        if (file_exists($json_file))
            $this->ground->load_schema_from_file($json_file);
 * 
 */
    }
}
