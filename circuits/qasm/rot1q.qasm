OPENQASM 2.0;
include "qelib1.inc";
qreg q[1];
creg c[1];

rx(1.0) q[0];
rz(0.7) q[0];
ry(1.2) q[0];
