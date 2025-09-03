// simple print
stdout [Hello World!]

;; Variables
new name string = "John Doe"
new age int = 100
new is_alive bool = true
stdout [{name} is {age} years old and is {is_alive}]

;; this will be a comment
; So will be this, but the ending part is important ;

;
And ofc this can be used as
a way for multiline comments
;

// array
// ✅ Inferred type arrays
new numbers[] = [1, 2, 3, 4, 5]
new names[] = ["Alice", "Bob"]

// ✅ Explicit type with size
new scores{float}[10]
new buffer{int}[100]
