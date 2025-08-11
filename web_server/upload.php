<?php

$uploadDir = __DIR__ . '/uploads/';
if (!is_dir($uploadDir)) {
  mkdir($uploadDir, 0755, true);
}

function public_url_for($basename) {
  $scheme = (!empty($_SERVER['HTTPS']) && $_SERVER['HTTPS'] === 'on') ? 'https' : 'http';
  $host   = $_SERVER['HTTP_HOST'];
  $base   = rtrim(dirname($_SERVER['PHP_SELF']), '/\\');
  return $scheme . '://' . $host . $base . '/uploads/' . $basename;
}

function new_filename() {
  return 'crack_' . date('Ymd_His') . '_' . bin2hex(random_bytes(3)) . '.jpg';
}

if (!empty($_FILES['file']['tmp_name'])) {
  $name = new_filename();
  $dest = $uploadDir . $name;

  if (move_uploaded_file($_FILES['file']['tmp_name'], $dest)) {
    echo public_url_for($name);
    exit;
  }
  http_response_code(500);
  echo 'Upload failed';
  exit;
}

$raw = file_get_contents('php://input');
if ($raw !== false && strlen($raw) > 0) {
  $name = new_filename();
  $dest = $uploadDir . $name;

  if (file_put_contents($dest, $raw) !== false) {
    echo public_url_for($name);
    exit;
  }
  http_response_code(500);
  echo 'Write failed';
  exit;
}

http_response_code(400);
echo 'No data received';
