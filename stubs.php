<?php
function modifiersOf($thing) {
    $result = [];
    
    $modifiers = 
        $thing->getModifiers();
    
    if ($modifiers & ReflectionMethod::IS_FINAL)      $result[] = "final";
    if ($modifiers & ReflectionMethod::IS_ABSTRACT)   $result[] = "abstract";
    if ($modifiers & ReflectionMethod::IS_PUBLIC)     $result[] = "public";
    if ($modifiers & ReflectionMethod::IS_PROTECTED)  $result[] = "protected";
    if ($modifiers & ReflectionMethod::IS_PRIVATE)    $result[] = "private";
    if ($modifiers & ReflectionMethod::IS_STATIC)     $result[] = "static";

    return implode(" ", $result);
}
function argumentOf($parameter) {
    $argument = [];
    
    if ($parameter->isPassedByReference()) {
        $argument[] = "&";
    }
    
    if ($parameter->isArray()) {
        $argument[] = "array";
    }
    
    if ($parameter->isCallable()) {
        $argument[] = "callable";
    }
    
    if ($parameter->getClass()) {
        $argument[] = 
            classOf($parameter->getClass());
    }
    
    $argument[] = sprintf("$%s", $parameter->getName());
    
    if ($parameter->isDefaultValueAvailable()) {
        $argument[] = sprintf
            ("= %s", $parameter->getDefaultValue());
    }
    
    return implode(" ", $argument);
}
function argumentsOf($thing) {
    $arguments = [];
    
    foreach ($thing->getParameters() as $parameter) {
        $arguments[] = argumentOf($parameter);
    }
    
    return implode(", ", $arguments);
}
function namespaceOf($thing) { return $thing->getNamespaceName(); }
function nameOf($thing)      { return $thing->getName(); }
function classOf($thing) {
    $namespace = namespaceOf($thing);
    $name      = nameOf($thing);

    if (strpos($name, $namespace) === 0) {
        return substr($name, strlen($namespace)+1);
    } else return $name;
}

$extension = new ReflectionExtension("jitfu");

foreach ($extension->getClasses() as $class) {
    $handle = fopen(sprintf(
        "stubs/%s.php", classOf($class)), "w+");
    
    fprintf($handle, "<?php\n");
    fprintf($handle, "namespace %s {\n", namespaceOf($class));
    fprintf($handle, "\tclass %s {\n", classOf($class));
    foreach ($class->getMethods() as $method) {
        fprintf(
            $handle,
            "\t\t%s function %s (%s) {}\n", 
            modifiersOf($method),
            nameOf($method),
            argumentsOf($method));
    }
    fprintf($handle, "\t}\n");
    fprintf($handle, "}\n");
}
?>
