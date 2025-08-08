<?php
$target_dir = "uploads/";
if (!file_exists($target_dir)) {
    mkdir($target_dir, 0755, true);
}
$target_file = $target_dir . basename($_FILES["file"]["name"]);
if (move_uploaded_file($_FILES["file"]["tmp_name"], $target_file)) {
    echo "http://your_server_ip/uploads/" . basename($_FILES["file"]["name"]);
} else {
    echo "Upload failed";
}
?>
