namespace WinFormsApp1
{
    partial class SerialPortForm
    {
        /// <summary>
        /// Required designer variable.
        /// </summary>
        private System.ComponentModel.IContainer components = null;

        /// <summary>
        /// Clean up any resources being used.
        /// </summary>
        /// <param name="disposing">true if managed resources should be disposed; otherwise, false.</param>
        protected override void Dispose(bool disposing)
        {
            if (disposing && (components != null))
            {
                components.Dispose();
            }
            base.Dispose(disposing);
        }

        #region Windows Form Designer generated code

        /// <summary>
        /// Required method for Designer support - do not modify
        /// the contents of this method with the code editor.
        /// </summary>
        private void InitializeComponent()
        {
            label1 = new Label();
            label2 = new Label();
            cmbPorts = new ComboBox();
            txtBaudRate = new TextBox();
            btnRefreshPorts = new Button();
            btnOpenPort = new Button();
            btnClosePort = new Button();
            rtbLog = new RichTextBox();
            pnlControl = new Panel();
            pnlHeader = new Panel();
            lblTitle = new Label();
            pnlControl.SuspendLayout();
            pnlHeader.SuspendLayout();
            SuspendLayout();
            // 
            // pnlHeader
            // 
            pnlHeader.BackColor = Color.White;
            pnlHeader.Controls.Add(lblTitle);
            pnlHeader.Dock = DockStyle.Top;
            pnlHeader.Location = new Point(0, 0);
            pnlHeader.Name = "pnlHeader";
            pnlHeader.Padding = new Padding(20);
            pnlHeader.Size = new Size(800, 60);
            pnlHeader.TabIndex = 8;
            // 
            // lblTitle
            // 
            lblTitle.AutoSize = true;
            lblTitle.Font = new Font("微软雅黑", 14, FontStyle.Bold);
            lblTitle.ForeColor = UIHelper.Colors.Primary;
            lblTitle.Location = new Point(20, 15);
            lblTitle.Name = "lblTitle";
            lblTitle.Size = new Size(120, 25);
            lblTitle.TabIndex = 0;
            lblTitle.Text = "串口连接";
            // 
            // pnlControl
            // 
            pnlControl.BackColor = Color.White;
            pnlControl.Controls.Add(label1);
            pnlControl.Controls.Add(label2);
            pnlControl.Controls.Add(cmbPorts);
            pnlControl.Controls.Add(txtBaudRate);
            pnlControl.Controls.Add(btnRefreshPorts);
            pnlControl.Controls.Add(btnOpenPort);
            pnlControl.Controls.Add(btnClosePort);
            pnlControl.Dock = DockStyle.Right;
            pnlControl.Location = new Point(550, 60);
            pnlControl.Name = "pnlControl";
            pnlControl.Padding = new Padding(20);
            pnlControl.Size = new Size(250, 390);
            pnlControl.TabIndex = 8;
            // 
            // label1
            // 
            label1.AutoSize = true;
            label1.Location = new Point(20, 20);
            label1.Name = "label1";
            label1.Size = new Size(44, 17);
            label1.TabIndex = 0;
            label1.Text = "串口号";
            // 
            // label2
            // 
            label2.AutoSize = true;
            label2.Location = new Point(20, 80);
            label2.Name = "label2";
            label2.Size = new Size(44, 17);
            label2.TabIndex = 1;
            label2.Text = "波特率";
            // 
            // cmbPorts
            // 
            cmbPorts.FormattingEnabled = true;
            cmbPorts.Location = new Point(20, 40);
            cmbPorts.Name = "cmbPorts";
            cmbPorts.Size = new Size(210, 25);
            cmbPorts.TabIndex = 2;
            // 
            // txtBaudRate
            // 
            txtBaudRate.Location = new Point(20, 100);
            txtBaudRate.Name = "txtBaudRate";
            txtBaudRate.Size = new Size(210, 23);
            txtBaudRate.TabIndex = 3;
            // 
            // btnRefreshPorts
            // 
            btnRefreshPorts.Location = new Point(20, 150);
            btnRefreshPorts.Name = "btnRefreshPorts";
            btnRefreshPorts.Size = new Size(210, 35);
            btnRefreshPorts.TabIndex = 4;
            btnRefreshPorts.Text = "刷新串口";
            btnRefreshPorts.UseVisualStyleBackColor = true;
            btnRefreshPorts.Click += btnRefreshPorts_Click;
            // 
            // btnOpenPort
            // 
            btnOpenPort.Location = new Point(20, 200);
            btnOpenPort.Name = "btnOpenPort";
            btnOpenPort.Size = new Size(210, 35);
            btnOpenPort.TabIndex = 5;
            btnOpenPort.Text = "打开串口";
            btnOpenPort.UseVisualStyleBackColor = true;
            btnOpenPort.Click += btnOpenPort_Click;
            // 
            // btnClosePort
            // 
            btnClosePort.Location = new Point(20, 250);
            btnClosePort.Name = "btnClosePort";
            btnClosePort.Size = new Size(210, 35);
            btnClosePort.TabIndex = 6;
            btnClosePort.Text = "关闭串口";
            btnClosePort.UseVisualStyleBackColor = true;
            btnClosePort.Click += btnClosePort_Click;
            // 
            // rtbLog
            // 
            rtbLog.BackColor = Color.FromArgb(245, 245, 245);
            rtbLog.Dock = DockStyle.Fill;
            rtbLog.Location = new Point(0, 60);
            rtbLog.Name = "rtbLog";
            rtbLog.ReadOnly = true;
            rtbLog.Size = new Size(550, 390);
            rtbLog.TabIndex = 7;
            rtbLog.Text = "";
            // 
            // SerialPortForm
            // 
            AutoScaleDimensions = new SizeF(7F, 17F);
            AutoScaleMode = AutoScaleMode.Font;
            ClientSize = new Size(800, 450);
            Controls.Add(pnlControl);
            Controls.Add(rtbLog);
            Controls.Add(pnlHeader);
            Name = "SerialPortForm";
            Text = "串口连接";
            FormClosed += SerialPortForm_FormClosed;
            Load += SerialPortForm_Load;
            pnlControl.ResumeLayout(false);
            pnlControl.PerformLayout();
            pnlHeader.ResumeLayout(false);
            pnlHeader.PerformLayout();
            ResumeLayout(false);
        }

        #endregion

        private Label label1;
        private Label label2;
        private ComboBox cmbPorts;
        private TextBox txtBaudRate;
        private Button btnRefreshPorts;
        private Button btnOpenPort;
        private Button btnClosePort;
        private RichTextBox rtbLog;
        private Panel pnlControl;
        private Panel pnlHeader;
        private Label lblTitle;
    }
}