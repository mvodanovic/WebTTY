<?php

namespace Config\Specifics;

final class Data
{
    private static $_data = array(
        'APP_REWRITE_BASE' => '/',
        'SHOW_DEBUG_INFO' => true,
        'INCLUDE_FW_SIGNATURE' => true,
        'DISPLAY_ERRORS' => true,
        'ERROR_REPORTING' => -1,
        'DEFAULT_CTL' => 'ShowTerminal',
    );

    public static function GetItem($key)
    {
        if (array_key_exists($key, self::$_data)) {
            return self::$_data[$key];
        } else {
            return null;
        }
    }

    private function __construct() {}
}
