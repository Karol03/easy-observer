namespace examples
{
extern void simple_example();
extern void advanced_example();
}  // namespace examples


int main()
{
    // examples::simple_example();  // call examples::simple_example() from examples/simple_example.cpp
    examples::advanced_example();  // call examples::advanced_example() from examples/advanced_example.cpp
}
