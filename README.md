# pure

Implementation of pure lisp.  
(But already it's not 'pure'.pure has many optional functions.)

## Build & Test

```sh
$ make
$ make test
$ ./test
```

## Usage

###### 1.REPL interface

```sh
$ ./pure
> 1
1
>
```

###### 2. Running source file

```sh
$ ./pure [filename]
```

###### 3. Running from stdin source code

```sh
$ echo "(print 1)" | ./pure
1
```

## Spec

#### Special Form

Implements the following Special Form.

 * if     
 * quote  
 * lambda 
 * define

#### Primitive Functions

Implements the following functions.

 * atom  
 * eq    
 * car   
 * cdr   
 * cons  
 * print 
 * \+     
 * \-     
 * \*     
 * /     
 * %     
 * <     
 * \>     
 * <=    
 * \>=    

#### Garbage Collection

pure has adopted a conservative Mark & Sweep GC.

#### Scope

pure has adopted a lexical scope.

## Example

Implementation of [tak function](https://en.wikipedia.org/wiki/Tak_(function) is as follows:

```lisp
(define tak (lambda (x y z)
  (if (<= x y)
    y
    (tak
      (tak (- x 1) y z)
      (tak (- y 1) z x)
      (tak (- z 1) x y)))))

(print (tak 10 5 0))
```

## License

MIT
