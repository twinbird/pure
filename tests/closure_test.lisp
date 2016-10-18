(define adder-gen
  (lambda (x)
	(lambda (y)
	  (+ x y))))

(define inc (adder-gen 1))
(define 2up (adder-gen 2))

(print (inc 1))
(print (inc 2))
(print (2up 1))
(print (2up 2))
