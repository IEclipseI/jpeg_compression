import java.io.File
import java.io.FileFilter


fun main() {
    val initial = File("./tmp").listFiles(FileFilter { it.name.endsWith(".decoded") && it.name.contains(args[0]) })
        .map { it.length() }
        .sum()
        .let {
            println("Initial summary size: $it")
            it
        }
    val compressed = File("./tmp").listFiles(FileFilter { it.name.endsWith(".coded") && it.name.contains(args[0]) })
        .map { it.length() }
        .sum()
        .let {
            println("Compressed summary size: $it")
            it
        }


    println("Ratio: ${compressed.toDouble() / initial}")
    println("Diff : ${compressed.toDouble() / initial - 1}")
}

main()