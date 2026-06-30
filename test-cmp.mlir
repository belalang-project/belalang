bir.func @main() {
  %0 = bir.constant #bir.float<0.00> : !bir.float
  %1 = bir.constant #bir.int<1> : !bir.int

  %2 = bir.cmp lt %0, %1 : !bir.int

  bir.return
}
