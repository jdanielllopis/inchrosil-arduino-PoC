import tkinter as tk
from tkinter import scrolledtext
import subprocess
import threading


class TerminalGUI:
    def __init__(self, root):
        self.root = root
        self.root.title("Terminal GUI")
        self.root.geometry("800x600")
        
        # Create main frame
        main_frame = tk.Frame(root, padx=10, pady=10)
        main_frame.pack(fill=tk.BOTH, expand=True)
        
        # Create terminal output area
        terminal_label = tk.Label(main_frame, text="Terminal Output:", anchor="w")
        terminal_label.pack(fill=tk.X, pady=(0, 5))
        
        self.terminal_output = scrolledtext.ScrolledText(
            main_frame,
            wrap=tk.WORD,
            width=80,
            height=20,
            bg="black",
            fg="white",
            font=("Courier", 10)
        )
        self.terminal_output.pack(fill=tk.BOTH, expand=True, pady=(0, 10))
        
        # Create command input area
        input_frame = tk.Frame(main_frame)
        input_frame.pack(fill=tk.X, pady=(0, 10))
        
        command_label = tk.Label(input_frame, text="Command:")
        command_label.pack(side=tk.LEFT, padx=(0, 5))
        
        self.command_entry = tk.Entry(input_frame)
        self.command_entry.pack(side=tk.LEFT, fill=tk.X, expand=True, padx=(0, 5))
        self.command_entry.bind("<Return>", lambda e: self.run_terminal_command())
        
        # Create buttons frame
        button_frame = tk.Frame(main_frame)
        button_frame.pack(fill=tk.X)
        
        # Terminal button
        self.terminal_button = tk.Button(
            button_frame,
            text="Run Command",
            command=self.run_terminal_command,
            bg="#4CAF50",
            fg="white",
            padx=20,
            pady=10,
            font=("Arial", 11, "bold")
        )
        self.terminal_button.pack(side=tk.LEFT, padx=(0, 10))
        
        # Clear button
        clear_button = tk.Button(
            button_frame,
            text="Clear",
            command=self.clear_terminal,
            bg="#2196F3",
            fg="white",
            padx=20,
            pady=10,
            font=("Arial", 11, "bold")
        )
        clear_button.pack(side=tk.LEFT, padx=(0, 10))
        
        # Close button
        close_button = tk.Button(
            button_frame,
            text="Close",
            command=self.close_application,
            bg="#f44336",
            fg="white",
            padx=20,
            pady=10,
            font=("Arial", 11, "bold")
        )
        close_button.pack(side=tk.RIGHT)
        
    def run_terminal_command(self):
        """Execute the command entered in the command entry"""
        command = self.command_entry.get().strip()
        if not command:
            return
            
        self.terminal_output.insert(tk.END, f"$ {command}\n", "command")
        self.terminal_output.tag_config("command", foreground="#00ff00")
        self.terminal_output.see(tk.END)
        
        # Run command in separate thread to avoid blocking GUI
        thread = threading.Thread(target=self._execute_command, args=(command,))
        thread.daemon = True
        thread.start()
        
        # Clear command entry
        self.command_entry.delete(0, tk.END)
    
    def _execute_command(self, command):
        """Execute command and display output"""
        try:
            process = subprocess.Popen(
                command,
                shell=True,
                stdout=subprocess.PIPE,
                stderr=subprocess.PIPE,
                text=True
            )
            
            stdout, stderr = process.communicate()
            
            if stdout:
                self.root.after(0, self._append_output, stdout, "stdout")
            if stderr:
                self.root.after(0, self._append_output, stderr, "stderr")
            
            if process.returncode != 0:
                self.root.after(0, self._append_output, 
                              f"Command exited with code {process.returncode}\n", "error")
            
        except Exception as e:
            self.root.after(0, self._append_output, f"Error: {str(e)}\n", "error")
    
    def _append_output(self, text, tag):
        """Append text to terminal output"""
        self.terminal_output.insert(tk.END, text, tag)
        self.terminal_output.tag_config("stdout", foreground="white")
        self.terminal_output.tag_config("stderr", foreground="#ff6b6b")
        self.terminal_output.tag_config("error", foreground="#ff0000")
        self.terminal_output.see(tk.END)
    
    def clear_terminal(self):
        """Clear the terminal output"""
        self.terminal_output.delete(1.0, tk.END)
    
    def close_application(self):
        """Close the application"""
        self.root.quit()
        self.root.destroy()


if __name__ == "__main__":
    root = tk.Tk()
    app = TerminalGUI(root)
    root.mainloop()
