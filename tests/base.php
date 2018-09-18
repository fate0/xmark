<?php

function xmark_var(&$var, $recursive=true) {
    if (is_string($var)) {
        xmark($var);
    } elseif (is_array($var) && $recursive) {
        foreach ($var as $key => &$value) {
            xmark_var($value, $recursive);
        }
    }
}


function xcheck_var($var, $recursive=true) {
    if (is_string($var)) {
        return xcheck($var);
    } elseif (is_array($var) && $recursive) {
        foreach ($var as $key => &$value) {
            if (xcheck_var($value, $recursive)) return true;
        }
    }

    return false;
}


function echo_handler($string) {
    if (is_string($string) && xcheck($string)) {
        echo "echo_handler:xmark: ".json_encode($string)." ".xcheck($string)."\n";
    }
}

function exit_handler($string) {
    if (is_string($string) && xcheck($string)) {
        echo "exit_handler:xmark: ".json_encode($string)." ".xcheck($string)."\n";
    }
}

function init_user_dynamic_call_handler($funcname) {
    if (is_string($funcname)) {
        if (xcheck($funcname)) {
            echo "init_user_dynamic_call_handler:xcheck: ".json_encode($funcname)." ".xcheck($funcname)."\n";
        }
    } else if (is_array($funcname)) {
        if (xcheck($funcname[0])) {
            echo "init_user_dynamic_call_handler:xcheck: ".json_encode($funcname[0])." ".xcheck($funcname[0])."\n";
        }
        if (xcheck($funcname[1])) {
            echo "init_user_dynamic_call_handler:xcheck: ".json_encode($funcname[1])." ".xcheck($funcname[1])."\n";
        }
    }
}

function include_or_eval_handler($param) {
    if (xcheck($param)) {
        echo "include_or_eval_handler:xmark: ".json_encode($param)." ".xcheck($param)."\n";
    }
}

function concat_handler($param1, $param2) {
    $result = $param1.$param2;

    if (xcheck($param1) || xcheck($param2)) {
        xmark($result);
    }

    return $result;
}

function rope_end_handler($params) {
    $result = implode($params);
    if (xcheck_var($params)) {
        xmark($result);
    }
    return $result;
}


function do_fcall($call, $params) {
    // do nothing
}

function do_icall($call, $params) {
    // do nothing
}

function do_fcall_by_name($call, $params) {
    // do nothing
}


xregister_opcode_callback(XMARK_ECHO, 'echo_handler');
xregister_opcode_callback(XMARK_EXIT, 'exit_handler');
xregister_opcode_callback(XMARK_INIT_METHOD_CALL, 'init_user_dynamic_call_handler');
xregister_opcode_callback(XMARK_INIT_USER_CALL, 'init_user_dynamic_call_handler');
xregister_opcode_callback(XMARK_INIT_DYNAMIC_CALL, 'init_user_dynamic_call_handler');
xregister_opcode_callback(XMARK_INCLUDE_OR_EVAL, "include_or_eval_handler");
xregister_opcode_callback(XMARK_CONCAT, 'concat_handler');
xregister_opcode_callback(XMARK_FAST_CONCAT, 'concat_handler');
xregister_opcode_callback(XMARK_ASSIGN_CONCAT, "concat_handler");
xregister_opcode_callback(XMARK_ROPE_END, 'rope_end_handler');
xregister_opcode_callback(XMARK_DO_FCALL, 'do_fcall');
xregister_opcode_callback(XMARK_DO_ICALL, 'do_icall');
xregister_opcode_callback(XMARK_DO_FCALL_BY_NAME, 'do_fcall_by_name');

?>