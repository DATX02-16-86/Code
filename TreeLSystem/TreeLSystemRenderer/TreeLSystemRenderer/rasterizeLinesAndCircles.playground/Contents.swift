//: Playground - noun: a place where people can play

import Cocoa

typealias Bit = Character
typealias Line = [Bit]
typealias Bitmap = [Line]

func toString(bitmap: Bitmap) -> String {
  return bitmap.reduce("") { string, line in
    string + line.reduce("") { $0 + String($1) } + "\n"
  }
}

func rasterizeLine(x0 x0: Int, y0: Int, x1: Int, y1: Int) -> Bitmap {
  var x0 = x0, y0 = y0, x1 = x1, y1 = y1
  let negDirX = x0 > x1
  let negDirY = y0 > y1
  if negDirX { swap(&x0, &x1) }
  if negDirY { swap(&y0, &y1) }
  
  var dx = x1 - x0
  var dy = y1 - y0
  let tall = dx < dy
  if tall { swap(&x0, &y0); swap(&x1, &y1); swap(&dx, &dy) }
  
  var line = Bitmap(count: tall ? dx+1 : dy+1, repeatedValue: Line(count: tall ? dy+1 : dx+1, repeatedValue: "."))
  var D = dy - dx
  var y = 0
  for x in 0 ... dx {
    let xi: Int
    let yi: Int
    xi = negDirX ? (tall ? dy-y : dx-x) : (tall ? y : x)
    yi = negDirY ? (tall ? x : y) : (tall ? dx-x : dy-y)
    line[yi][xi] = x == 0 ? "O" : "X"
    if D >= 0 {
      y += 1
      D -= dx
    }
    D += dy
  }
  return line
}

func rasterizeCircle(radius r: Int) -> Bitmap {
  var x = r, y = 0
  var dx = 1-2*r, dy = 1
  var re = 0
  var circle = Bitmap(count: r*2+1, repeatedValue: Line(count: r*2+1, repeatedValue: "."))
  while x >= y {
    let a1 = [(x, -y), (-x, y), (-y, -x),  (y, x)]
    let a2 = [(x, y), (-x, -y), (-y, x), (y, -x)]
    for (px, py) in a1 {
      circle[py + r][px + r] = circle[py + r][px + r] == "X" ? "O" : "X"
    }
    if y > 0 {
      for (px, py) in a2 {
        circle[py + r][px + r] = circle[py + r][px + r] == "X" ? "O" : "X"
      }
    }
    y += 1
    re += dy
    dy += 2
    if 2*re+dx > 0 {
      x -= 1
      re += dx
      dx += 2
    }
  }
  return circle
}

//print(toString(rasterizeCircle(radius: 100)))

//print(toString(rasterizeLine(x0: 0, y0: 0, x1: 0, y1: 5)))
//print(toString(rasterizeLine(x0: 0, y0: 0, x1: 3, y1: 5)))
//print(toString(rasterizeLine(x0: 0, y0: 0, x1: 4, y1: 3)))
//print(toString(rasterizeLine(x0: 0, y0: 0, x1: 5, y1: 0)))
//print(toString(rasterizeLine(x0: 0, y0: 0, x1: 4, y1: -3)))
//print(toString(rasterizeLine(x0: 0, y0: 0, x1: 3, y1: -4)))
//print(toString(rasterizeLine(x0: 0, y0: 0, x1: 0, y1: -5)))
//print(toString(rasterizeLine(x0: 0, y0: 0, x1: -3, y1: -4)))
//print(toString(rasterizeLine(x0: 0, y0: 0, x1: -4, y1: -3)))
//print(toString(rasterizeLine(x0: 0, y0: 0, x1: -5, y1: 0)))
//print(toString(rasterizeLine(x0: 0, y0: 0, x1: -4, y1: 3)))
//print(toString(rasterizeLine(x0: 0, y0: 0, x1: -3, y1: 4)))

let steps = 360
let radius = 5.0
let lengthWidthRatio = 1
for i in 0 ..< steps {
  let rad = 2*M_PI*Double(i)/Double(steps)
  let (x0, y0) = (Int(-radius*cos(rad)), Int(-radius*sin(rad)))
  let (x1, y1) = (Int(radius*cos(rad)), Int(radius*sin(rad)))
  print(toString(rasterizeLine(x0: x0, y0: y0, x1: x1, y1: y1)))
  //  print(toString(rasterizeLine(x0: 0, y0: 0, x1: -dy*lengthWidthRatio, y1: dx*lengthWidthRatio)))
}
