--TEST--
Bug GH-16032: Various NULL pointer dereferencements in ldap_modify_batch()
--EXTENSIONS--
ldap
--FILE--
<?php

/* We are assuming 3333 is not connectable */
$ldap = ldap_connect('ldap://127.0.0.1:3333');
$valid_dn = "cn=userA,something";

$modification_missing_attrib_key = [
    [
        "modtype" => LDAP_MODIFY_BATCH_ADD,
        "values"  => ["value1"],
    ],
];
try {
    var_dump(ldap_modify_batch($ldap, $valid_dn, $modification_missing_attrib_key));
} catch (Throwable $e) {
    echo $e::class, ': ', $e->getMessage(), PHP_EOL;
}

?>
--EXPECT--
ValueError: ldap_modify_batch(): Argument #3 ($modifications_info) a modification entry must contain the "attrib" option
