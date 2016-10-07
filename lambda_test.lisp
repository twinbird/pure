(define cadr
  (lambda (x)
	(car
	  (cdr x))))
(cadr
  (quote (1 2 3)))

