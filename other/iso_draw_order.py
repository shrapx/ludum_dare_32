# draw isometric order

def get_iso(square):
  area = square*square
  depth = 0
  num_last_depth = 0
  pos = 0
  arr = []
  while depth < (square*2):
    
    if pos < square:
      num_last_depth = (depth * square)
      depth += 1
      pos = num_last_depth
    else:
      pos = pos - square + 1

    if pos < area:
      if pos not in arr:
        arr += [pos]
        
  return arr

a = get_iso(4)
print( a )
print( len(a) )

a = get_iso(8)
print( a )
print( len(a) )

a = get_iso(16)
print( a )
print( len(a) )
  

