<?php

namespace Application\Classes;

use \WebFW\Core\SessionHandler;
use \Config\Specifics\Data;

class TTYClient
{
    protected static $instance = null;
    protected $socket;
    protected $socketAddress;

    const DAEMON_SOCKET_ADDRESS = '/tmp/webttyd.socket';
    const CMD_ESTABLISH_CONNECTION = 'c';
    const CMD_WRITE = 'w';
    const CMD_READ = 'r';

    protected $lastErrorCode = null;
    protected $connectionIsEstablished = false;

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
        if ($this->obtainSocketAddress() === false) {
            return;
        }

        $this->connectToSocket();
    }

    protected function obtainSocketAddress()
    {
        if (file_exists(static::DAEMON_SOCKET_ADDRESS) === false) {
            $this->lastErrorCode = 2; /// Same as if socket_connect fails
            return false;
        }

        if (($socket = socket_create(AF_UNIX, SOCK_STREAM,  0)) === false) {
            $this->lastErrorCode = socket_last_error();
            return false;
        }

        if (socket_connect($socket, static::DAEMON_SOCKET_ADDRESS) === false) {
            $this->lastErrorCode = socket_last_error();
            return false;
        }

        $this->write(static::CMD_ESTABLISH_CONNECTION, $socket);
        $this->write(session_id(), $socket);

        $this->socketAddress = $this->read($socket);

        socket_close($socket);

        return true;
    }

    protected function connectToSocket()
    {
        if (file_exists($this->socketAddress) === false) {
            $this->lastErrorCode = 2; /// Same as if socket_connect fails
            return false;
        }

        if (($this->socket = socket_create(AF_UNIX, SOCK_STREAM,  0)) === false) {
            $this->lastErrorCode = socket_last_error();
            return;
        }

        if (socket_connect($this->socket, $this->socketAddress) === false) {
            $this->lastErrorCode = socket_last_error();
            return;
        }

        /// Params
        $params = Data::getItem('TTY_PARAMS');
        if (is_array($params)) {
            $paramsString = array();
            foreach ($params as $key => $value) {
                $paramsString[] = $key . '=' . $value;
            }
            $paramsString = implode(';', $paramsString);
        } else {
            $paramsString = $params . '';
        }
        $this->write($paramsString, $this->socket);

        $this->connectionIsEstablished = true;
    }

    /// TODO: error handling
    public function write($buffer, $socket = null)
    {
        if ($buffer == null) {
            return;
        }

        if ($socket === null) {
            if ($this->isConnectionEstablished() === false) {
                return;
            }

            $socket = $this->socket;
        }


        /// Append string terminator to string, preparation for C/C++ string handling
        $buffer .= "\0";

        /// PHP's equivalent for C's sizeof()
        $length = mb_strlen($buffer, '8bit');

        /// The length needs to be packed in the appropriate format for integers in C/C++
        /// It is assumed that integers in C have 32 bits (4 bytes)
        socket_send($socket, pack('l', $length), 4, null);

        /// Send the actual buffer
        socket_send($socket, $buffer, $length, null);
    }

    /// TODO: error handling
    public function read($socket = null)
    {
        if ($socket === null) {
            if ($this->isConnectionEstablished() === false) {
                return null;
            }

            $socket = $this->socket;
        }

        $buffer = null;

        $retRead = socket_recv($socket, $length, 4, null);
        if ($length === null) {
            return null;
        }

        /// The length needs to be unpacket from C/C++ integer format
        $length = unpack('l', $length);
        $length = reset($length);

        $retRead = socket_recv($socket, $buffer, $length, null);

        if ($retRead != $length) {
            $buffer = null;
            var_dump("buffer read error");
        }

        /// Remove the C/C++ string terminator from the end
        $buffer = rtrim($buffer);

        return $buffer;
    }

    public function getLastError()
    {
        if ($this->lastErrorCode === null) {
            return null;
        }

        return socket_strerror($this->lastErrorCode);
    }

    public function isConnectionEstablished()
    {
        return $this->connectionIsEstablished;
    }

    public function disconnect()
    {
        if ($this->isConnectionEstablished()) {
            $this->connectionIsEstablished = false;
            socket_close($this->socket);
        }
    }
}
