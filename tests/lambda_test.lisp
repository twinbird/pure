(define cadr
  (lambda (x)
	(car
	  (cdr x))))

(print 
  (cadr 
	(quote (1 2 3))))
