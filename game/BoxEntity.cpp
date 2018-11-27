﻿#include "Game.h"
#include "BoxEntity.h"
#include "ShaderUtil.h"
#include "ObjFile.h"

BoxEntity::BoxEntity()
{
    numTriangles = 0;
}

BoxEntity::~BoxEntity()
{
}

bool BoxEntity::Init()
{
    std::vector<Vertex> vertex;
    std::vector<uint32_t> modelIndex;
    LoadObjModel(vertex, modelIndex, "./model.obj");
    numTriangles = modelIndex.size() / 3;

    // プログラムオブジェクト作成
    program = ShaderUtil::LoadProgram("lambert.vert", "lambert.frag");
    glUseProgram(program);
    // 頂点配列オブジェクト
    glGenVertexArrays(1, &vertexArrayObj);
    glBindVertexArray(vertexArrayObj);
    // 頂点バッファ
    glGenBuffers(1, &vertexBufferObj);
    glBindBuffer(GL_ARRAY_BUFFER, vertexBufferObj);
    glBufferData(GL_ARRAY_BUFFER, vertex.size() * sizeof(Vertex), vertex.data(), GL_STATIC_DRAW);
    // インデックスバッファ
    glGenBuffers(1, &indexBufferObj);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBufferObj);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, modelIndex.size() * sizeof(uint32_t), modelIndex.data(), GL_STATIC_DRAW);
    // 頂点バッファオブジェクトの頂点シェーダ入力への紐付け
    glBindAttribLocation(program, 0, "position");
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), reinterpret_cast<void*>(0));
    glBindAttribLocation(program, 1, "normal");
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), reinterpret_cast<void*>(12));
    // uniform変数の設定
    vModelDiffuse = glm::vec4(1.0f, 0.0f, 0.0f, 1.0f); // (R,G,B,A) = (1, 0, 0, 1) : Red
    // uniform変数
    uidModel        = glGetUniformLocation(program, "mModel");          // モデリング変換行列
    uidView         = glGetUniformLocation(program, "mView");           // 視野変換行列
    uidProjection   = glGetUniformLocation(program, "mProjection");     // 射影変換行列
    uidLocalLight   = glGetUniformLocation(program, "vLocalLight");     // モデリング座標系におけるライト位置
    uidLocalCamera  = glGetUniformLocation(program, "vLocalCamera");    // モデリング座標系におけるカメラ位置
    uidModelDiffuse = glGetUniformLocation(program, "vModelDiffuse");   // モデルの拡散反射成分
    uidLightDiffuse = glGetUniformLocation(program, "vLightDiffuse");   // ライトの拡散反射成分
    uidWindowSize = glGetUniformLocation(program, "windowSize");        // ウィンドウサイズ
    uidTime = glGetUniformLocation(program, "time");                    // 経過時間
    uidMousePos = glGetUniformLocation(program, "mousePos");            // マウス位置
    glUniformMatrix4fv(uidView, 1, GL_FALSE, glm::value_ptr(owner->ViewMatrix()));
    glUniformMatrix4fv(uidProjection, 1, GL_FALSE, glm::value_ptr(owner->ProjectionMatrix()));
    glUniform4fv(uidModelDiffuse, 1, glm::value_ptr(vModelDiffuse));
    glUniform4fv(uidLightDiffuse, 1, glm::value_ptr(owner->LightDiffuse()));
    glm::vec2 windowSize = owner->WindowSize();
    glUniform2f(uidWindowSize, windowSize.x, windowSize.y);

    return Entity::Init();
}

void BoxEntity::Update(const GameTime& time)
{
    // 現在の経過時間をセット
    glUniform1f(uidTime, time.TotalTime());
    // 現在のマウス位置をセット
    glm::vec2 mousePos = owner->MousePos();
    glUniform2f(uidMousePos, mousePos.x, mousePos.y);

    // 回転アニメーションの設定
    SetLocalTransform(glm::rotate(LocalTransform(), 1.0e-2f * glm::pi<float>(), glm::vec3(0, 1.0f, 0)));

    Entity::Update(time);
}

void BoxEntity::Render()
{
    // 背面カリングを有効にする
    glFrontFace(GL_CCW);
    glCullFace(GL_BACK);
    glEnable(GL_CULL_FACE);
    // デプスバッファを有効にする
    glClearDepth(1.0);
    glDepthFunc(GL_LESS);
    glEnable(GL_DEPTH_TEST);

    // シェーダの設定
    glUseProgram(program);
    glBindVertexArray(vertexArrayObj);
    // uniform変数の更新
    glm::mat4 mModel = WorldTransform();
    glm::mat4 mProjectionViewModel = owner->ProjectionMatrix() * owner->ViewMatrix() * mModel;
    glm::vec3 vLocalLight = glm::inverse(mModel) * owner->LightPosition();  // ワールド座標 -> モデリング座標系
    glm::vec3 vLocalCamera = glm::inverse(mModel) * owner->CameraPosition(); // ワールド座標 -> モデリング座標系
    glUniformMatrix4fv(uidModel, 1, GL_FALSE, glm::value_ptr(mModel));
    glUniform3fv(uidLocalLight, 1, glm::value_ptr(vLocalLight));
    glUniform3fv(uidLocalCamera, 1, glm::value_ptr(vLocalCamera));
    glDrawElements(GL_TRIANGLES, numTriangles * 3, GL_UNSIGNED_INT, 0);

    Entity::Render();
}

void BoxEntity::Release()
{
    glDeleteBuffers(1, &vertexBufferObj);
    glDeleteBuffers(1, &indexBufferObj);
    glDeleteBuffers(1, &vertexArrayObj);

    Entity::Release();
}
