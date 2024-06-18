;; Earl mode
(defconst earl-mode-syntax-table
  (with-syntax-table (copy-syntax-table)
    (modify-syntax-entry ?- ". 124b")
    (modify-syntax-entry ?* ". 23")
    (modify-syntax-entry ?\n "> b")
    (modify-syntax-entry ?' "\"")
    (modify-syntax-entry ?' ".")
    (syntax-table))
  "Syntax table for `earl-mode'.")

;; Function taken from:
;;  https://www.omarpolo.com/post/writing-a-major-mode.html
(defun earl-indent-line ()
  "Indent current line."
  (let (indent
        boi-p
        move-eol-p
        (point (point)))
    (save-excursion
      (back-to-indentation)
      (setq indent (car (syntax-ppss))
            boi-p (= point (point)))
      (when (and (eq (char-after) ?\n)
                 (not boi-p))
        (setq indent 0))
      (when boi-p
        (setq move-eol-p t))
      (when (or (eq (char-after) ?\))
                (eq (char-after) ?\}))
        (setq indent (1- indent)))
      (delete-region (line-beginning-position)
                     (point))
      (indent-to (* tab-width indent)))
    (when move-eol-p
      (move-end-of-line nil))))

(eval-and-compile
  (defconst earl-keywords
    '("if" "else" "while" "let" "void" "int"
      "str" "for" "fn" "return" "break"
      "struct" "char" "import" "in"
      ;; intrinsic functions
      "print" "assert"
      ;; member intrinsics
      "len" "rev" "nth" "append"
      ;; function attributes
      "world" "pub"
      )))

(defconst earl-highlights
  `((,(regexp-opt earl-keywords 'symbols) . font-lock-keyword-face)
    (,(rx (group "#" (zero-or-more nonl))) . font-lock-comment-face)))

;;;###autoload
(define-derived-mode earl-mode prog-mode "earl"
  "Major Mode for editing Earl source code."
  :syntax-table earl-mode-syntax-table
  (setq font-lock-defaults '(earl-highlights))
  (setq-local comment-start "#")
  (setq-local indent-tabs-mode nil)
  (setq-local tab-width 4)
  (setq-local indent-line-function #'earl-indent-line)
  (setq-local standard-indent 2))

;;;###autoload
(add-to-list 'auto-mode-alist '("\\.earl\\'" . earl-mode))

(provide 'earl-mode)
;; End Earl mode
