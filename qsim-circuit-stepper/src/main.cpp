int main() {
    Circuit c;
    c.add({ OpType::H, {0}, {} });
    c.add({ OpType::MEASURE, {0}, {} });

    Stepper stepper(c);

    while (!stepper.done()) {
        stepper.step();
    }
}
