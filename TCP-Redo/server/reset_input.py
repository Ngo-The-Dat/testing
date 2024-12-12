import os

def write_file_info(directory_path, output_file="input.txt"):
    try:
        # Check if the directory exists
        if not os.path.isdir(directory_path):
            raise NotADirectoryError(f"The path '{directory_path}' is not a directory or does not exist.")

        # Open the output file
        with open(output_file, "w") as file:
            # Iterate over items in the directory
            for entry in os.listdir(directory_path):
                entry_path = os.path.join(directory_path, entry)
                if os.path.isfile(entry_path):  # Only process files
                    file_size = os.path.getsize(entry_path)  # Get file size
                    file.write(f"{entry} {file_size}\n")

        print(f"File information written to '{output_file}' successfully.")

    except Exception as e:
        print(f"Error: {e}")

# Example usage
write_file_info("Files/")
