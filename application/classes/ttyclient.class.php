<?php

namespace Application\Classes;

use \WebFW\Core\SessionHandler;

class TTYClient
{
    protected static $instance = null;
    protected $socket;

    public static function getInstance()
    {
        if (static::$instance === null) {
            static::$instance = SessionHandler::get("tty");
            if (static::$instance === null) {
                static::$instance = new static();
                SessionHandler::set("tty", static::$instance);
            }
        }

        return static::$instance;
    }

    protected function __construct()
    {
        $this->socket = socket_create(AF_UNIX, SOCK_STREAM,  0);
        socket_connect($this->socket, '/tmp/webttyd.socket');
        $this->write('c');
        $this->write(session_id());
        var_dump($this->read());
        socket_close($this->socket);
    }

    /// TODO: error handling
    public function write($buffer)
    {
        if ($buffer == null) {
            return;
        }

        /// Append string terminator to string, preparation for C/C++ string handling
        $buffer .= "\0";

        /// PHP's equivalent for C's sizeof()
        $length = mb_strlen($buffer, '8bit');

        /// The length needs to be packed in the appropriate format for integers in C/C++
        /// It is assumed that integers in C have 32 bits (4 bytes)
        socket_send($this->socket, pack('l', $length), 4, null);

        /// Send the actual buffer
        socket_send($this->socket, $buffer, $length, null);
    }

    /// TODO: error handling
    public function read()
    {
        $buffer = null;

        $retRead = socket_recv($this->socket, $length, 4, null);

        /// The length needs to be unpacket from C/C++ integer format
        $length = unpack('l', $length);
        $length = reset($length);

        $retRead = socket_recv($this->socket, $buffer, $length, null);

        if ($retRead != $length) {
            $buffer = null;
            var_dump("buffer read error");
        }

        /// Remove the C/C++ string terminator from the end
        $buffer = rtrim($buffer);

        return $buffer;
    }
}
