void DrawESP(ESP esp) {
    auto screenHeight = esp.getHeight();
    auto screenWidth = esp.getWidth();
    if (!isConnected()) return;

    auto response = getData(screenWidth, screenHeight);
    if (!response.Success || response.PlayerCount <= 0) return;

    for (int i = 0; i < response.PlayerCount; i++) {
        auto player = response.Players[i];
        if (player.Head == Vector3::Zero() || player.Toe == Vector3::Zero()) continue;
        if (!Actived.activar) continue;

        float distance = player.Distancia;
        if (distance < 0.1f || distance > 150.0f) continue;

        // --- POSITION FIX ---
        // Calculate raw height and add a 15% buffer for better fit
        float headToFeetHeight = std::abs(player.Toe.Y - player.Head.Y);
        float boxHeight = headToFeetHeight * 1.15f; 
        float width = boxHeight * 0.48f;
        
        // Center the box horizontally on Head.X
        // Adjust Y so the top of the box is slightly above the head
        float boxY = player.Head.Y - (boxHeight * 0.12f);
        float boxX = player.Head.X - (width / 2.0f);

        Color currentBoxColor = player.IsCaido ? Color::Red() : pEspPlayer.espColor;

        // --- DISTANCE SCALING (Prevents "Ugly" thick lines at distance) ---
        float thickness = (distance > 70.0f) ? 1.0f : 1.5f;
        float fontSize = (distance > 70.0f) ? 10.0f : 12.0f;

        // --- RESTORED: LINE TYPES ---
        if (pEspPlayer.espLinha) {
            Vector3 lineStart;
            if (pEspPlayer.lineType == 0)      lineStart = Vector3(screenWidth / 2, 0, 0);
            else if (pEspPlayer.lineType == 1) lineStart = Vector3(screenWidth / 2, screenHeight / 2, 0);
            else if (pEspPlayer.lineType == 2) lineStart = Vector3(screenWidth / 2, screenHeight, 0);
            else                               lineStart = Vector3(screenWidth / 2, 0, 0);

            esp.DrawLine(currentBoxColor, 1.0f, lineStart, Vector3(player.Head.X, boxY, 0));
        }

        // --- RESTORED: BOX TYPES ---
        if (pEspPlayer.espCaixa) {
            Rect playerRect(boxX, boxY, width, boxHeight);

            if (pEspPlayer.boxType == 0) { // Stroke
                esp.DrawBox(currentBoxColor, thickness, playerRect);
            } 
            else if (pEspPlayer.boxType == 1) { // Filled
                Color fillColor = currentBoxColor;
                fillColor.a = 80;
                esp.DrawFilledRect(fillColor, Vector3(boxX, boxY, 0), width, boxHeight);
                esp.DrawBox(currentBoxColor, thickness, playerRect);
            } 
            else if (pEspPlayer.boxType == 2) { // Corner
                esp.DrawBox4Line(boxX, boxY, width, boxHeight, currentBoxColor, thickness);
            } 
            else if (pEspPlayer.boxType == 3) { // Rounded/Thick
                Color fillColor = currentBoxColor;
                fillColor.a = 60;
                esp.DrawFilledRect(fillColor, Vector3(boxX, boxY, 0), width, boxHeight);
                esp.DrawBox(currentBoxColor, thickness + 1.0f, playerRect);
            }

            // --- RESTORED: HEALTH BAR ---
            if (pEspPlayer.espInfo) {
                float healthBarWidth = (distance > 70.0f) ? 3.0f : 5.0f;
                float healthBarX = boxX - (healthBarWidth + 4);
                
                auto HealthMax = player.Name ? 100.0f : 200.0f;
                esp.DrawVerticalHealthBar(Vector2(healthBarX, boxY), boxHeight, HealthMax, player.Health, healthBarWidth, false);
            }
        }

        // --- RESTORED: TEXT INFO ---
        if (pEspPlayer.espInfo && !player.IsCaido) {
            auto PlayerID = std::to_string((int)(i + 1));
            auto PlayerDistance = std::to_string((int)distance) + "M";
            
            float nameY = boxY - (fontSize + 4);
            float distanceY = boxY + boxHeight + 5;
            Color outlineColor = Color(0, 0, 0, 200);

            // Draw Text with clean outline
            auto DrawOutlinedText = [&](const char* text, Vector2 pos) {
                esp.DrawText(outlineColor, text, Vector2(pos.x - 1, pos.y - 1), fontSize);
                esp.DrawText(outlineColor, text, Vector2(pos.x + 1, pos.y + 1), fontSize);
                esp.DrawText(currentBoxColor, text, pos, fontSize);
            };

            DrawOutlinedText(PlayerID.c_str(), Vector2(player.Head.X, nameY));
            DrawOutlinedText(PlayerDistance.c_str(), Vector2(player.Head.X, distanceY));
        }
    }
}
