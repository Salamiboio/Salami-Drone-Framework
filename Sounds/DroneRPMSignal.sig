AudioSignalResClass {
 Inputs {
  IOPItemInputClass {
   id 1
   name "AverageDroneRotorRPM"
   tl -255 -12
   children {
    3
   }
   value 17500
   valueMax 35000
  }
 }
 Ops {
  IOPItemOpInterpolateClass {
   id 3
   name "Interpolate 3"
   tl -28 -22
   children {
    2
   }
   inputs {
    ConnectionClass connection {
     id 1
     port 0
    }
   }
   "X max" 35000
   "Y min" 1
   "Y max" 4
   "Fade In Type" "S-curve"
   "Fade Out Type" "S-curve"
  }
 }
 Outputs {
  IOPItemOutputClass {
   id 2
   name "Rotor_P"
   tl 198 -0.756
   input 3
  }
 }
 compiled IOPCompiledClass {
  visited {
   5 7 6
  }
  ins {
   IOPCompiledIn {
    data {
     1 3
    }
   }
  }
  ops {
   IOPCompiledOp {
    data {
     1 2 2 0 0
    }
   }
  }
  outs {
   IOPCompiledOut {
    data {
     0
    }
   }
  }
  processed 3
  version 2
 }
}