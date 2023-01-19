//
//  ViewController.swift
//  Surf
//
//  Created by Jevin Sweval on 1/17/23.
//

import Cocoa
import ReactiveCocoa
import ReactiveSwift

extension Reactive where Base: CALayer {

    // Provided for cross-platform compatibility

    // public var transformValue: BindingTarget<CATransform3D> { return transform }
    // public var transformValues: Signal<CATransform3D, Never> { return floatValues }
}

class ViewController: NSViewController {
    @IBOutlet weak var label: NSTextField!
    @IBOutlet weak var slider: NSSlider!

    override func viewDidLoad() {
        super.viewDidLoad()

        let (signal, observer) = Signal<String, Never>.pipe()
        // signal
        slider.reactive.values
            .observeValues { value in print(value) }
        label.layer?.transform = CATransform3DMakeAffineTransform(CGAffineTransform(rotationAngle: 0.5 * 2 * 3.14))
        // label.layer?.reactive.transform <~ slider.reactive.values.map { value in CGAffineTransform(rotationAngle: value * 2 * 3.14) }
    }

    override var representedObject: Any? {
        didSet {
            // Update the view, if already loaded.
        }
    }

}
