import SceneKit
import QuartzCore

class GameViewController: NSViewController {
  
  @IBOutlet weak var gameView: SCNView!
  let height: CGFloat = 7.5
  let distance: CGFloat = 14
  
  override func awakeFromNib(){
    // create a new scene
    let scene = SCNScene()
    
    // create and add a camera to the scene
    let cameraNode = SCNNode()
    cameraNode.camera = SCNCamera()
    scene.rootNode.addChildNode(cameraNode)
    
    // create and add tree object
    let tree = TreeLSystemBridge()
    for vector in tree.voxels.map({ $0.vector }) {
      let cubeGeometry = SCNBox(width: 0.1, height: 0.1, length: 0.1, chamferRadius: 0)
      let material = SCNMaterial()
      let variance = CGFloat(Int(arc4random_uniform(50)) - 25) / 255.0
      material.diffuse.contents = NSColor(red: 139.0 / 255.0 + variance, green: 69.0 / 255.0 + variance, blue: 19.0 / 255.9 + variance, alpha: 1.0)
      cubeGeometry.materials = [material]
      let cubeNode = SCNNode(geometry: cubeGeometry)
      cubeNode.position = vector
      scene.rootNode.addChildNode(cubeNode)
    }
    
    // place the camera
    cameraNode.position = SCNVector3(x: 0, y: height, z: distance)
    var actionSequence: [SCNAction] = []
    actionSequence.reserveCapacity(360)
    for d in 0 ..< 360 {
      let r = CGFloat(2*d) * CGFloat(M_PI) / 360.0
      let moveAction = SCNAction.moveTo(SCNVector3Make(distance*sin(r), height, distance*cos(r)), duration: 0.05)
      let rotateAction = SCNAction.rotateToX(0, y: r, z: 0, duration: 0.05)
      actionSequence.append(SCNAction.group([moveAction, rotateAction]))
    }
    cameraNode.runAction(SCNAction.repeatActionForever(SCNAction.sequence(actionSequence)))
    
    // create and add an ambient light to the scene
    let ambientLightNode = SCNNode()
    ambientLightNode.light = SCNLight()
    ambientLightNode.light!.type = SCNLightTypeAmbient
    ambientLightNode.light!.color = NSColor.darkGrayColor()
    scene.rootNode.addChildNode(ambientLightNode)
    
    
    
    // set the scene to the view
    self.gameView.scene = scene
    
    // allows the user to manipulate the camera
    self.gameView.allowsCameraControl = true
    
    // configure the view
    self.gameView.backgroundColor = NSColor.whiteColor()
  }
  
}

//* -------------------------------------------------------------------------------------------- *//

//import SceneKit
//import QuartzCore
//
//class GameViewController: NSViewController {
//
//  @IBOutlet weak var gameView: SCNView!
//
//  override func awakeFromNib(){
//    // create a new scene
//    let scene = SCNScene()
//
//    // create and add a camera to the scene
//    let cameraNode = SCNNode()
//    cameraNode.camera = SCNCamera()
//    scene.rootNode.addChildNode(cameraNode)
//
//    // place the camera
//    cameraNode.position = SCNVector3(x: 0, y: 1, z: 5)
//
//    // create and add an ambient light to the scene
//    let ambientLightNode = SCNNode()
//    ambientLightNode.light = SCNLight()
//    ambientLightNode.light!.type = SCNLightTypeAmbient
//    ambientLightNode.light!.color = NSColor.darkGrayColor()
//    scene.rootNode.addChildNode(ambientLightNode)
//
//    // create and add tree object
//    let tree = TreeLSystemBridge()
//    let vertices = tree.vertices.map { $0.vector }
//    let indices = tree.indices.map { CInt($0.integerValue) }
//    let material = SCNMaterial()
//    material.diffuse.contents = NSColor(red: 139.0 / 255.0, green: 69.0 / 255.0, blue: 19.0 / 255.9, alpha: 1.0)
//    let geometrySrc = SCNGeometrySource(vertices: vertices, count: vertices.count)
//    let geometryEl = SCNGeometryElement(indices: indices, primitiveType: .Triangles)
//    let geometry = SCNGeometry(sources: [geometrySrc], elements: [geometryEl])
//    geometry.materials = [material]
//    scene.rootNode.addChildNode(SCNNode(geometry: geometry))
//
//    // set the scene to the view
//    self.gameView.scene = scene
//
//    // allows the user to manipulate the camera
//    self.gameView.allowsCameraControl = true
//
//    // configure the view
//    self.gameView.backgroundColor = NSColor.whiteColor()
//  }
//
//}

